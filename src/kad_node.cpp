#include "kadsim.h"

KadNode::KadNode(KadConf* conf, CBigNum id, std::string addr)
    : KadRoutable(id, KAD_ROUTABLE_NODE)
{
    this->conf = conf;
    this->id = id;
    this->verbose = false;
    this->addr = addr;
    if (!addr.empty()) {
        // FIXME use port
        this->httpclient = new jsonrpc::HttpClient(addr);
        this->kadc = new KadClient(*this->httpclient);
    }

    // The passphrase of the account is the hex of the node ID.
    this->eth_passphrase = id.ToString(16);
    try {
        this->eth_account = conf->geth.personal_newAccount(eth_passphrase);
        conf->geth.personal_unlockAccount(eth_account, eth_passphrase, 0);
    } catch (jsonrpc::JsonRpcException) {
        this->eth_account = "";
    }
}

KadNode::KadNode(KadConf* conf, CBigNum id) : KadNode(conf, id, "") {}

KadNode::~KadNode() {}

const std::string& KadNode::get_eth_account() const
{
    return eth_account;
}

/** Get the number of conns. */
int KadNode::get_n_conns()
{
    int i, total;

    total = 0;
    for (i = 1; i < (conf->n_bits + 1); i++)
        total += buckets[i].size();

    return total;
}

/** Insert a node in the routing table.
 *
 * @param node the node address to add in the buckets
 * @param contacted_us true if node contacted us (thus is alive)
 *
 * @return true if connection was added
 * @return false if connection was not added (e.g. because already present)
 */
bool KadNode::add_conn(KadNode* node, bool contacted_us)
{
    if (node->get_id() == get_id()) {
        std::cout << "cannot add itself " << get_id().ToString(16) << std::endl;
        return false;
    }

    CBigNum distance = distance_to(*node);
    int bit_length = distance.bit_length();

    std::list<KadNode*>& list = buckets[bit_length];
    std::list<KadNode*>::iterator it;
    bool found = false;
    for (it = list.begin(); it != list.end(); ++it) {
        if ((*it)->get_id() == node->get_id()) {
            found = true;
            break;
        }
    }

    if (found) {
        if (contacted_us) {
            // Move item to the head.
            list.splice(list.begin(), list, it);
            return false;
        } else {
            // Nothing to do.
            return false;
        }
    }

    // Add node in bucket.
    if (buckets[bit_length].size() < conf->k) {
        buckets[bit_length].push_back(node);
        return true;
    }

    return false;
}

std::list<KadNode*>
KadNode::find_nearest_nodes(KadRoutable routable, int amount)
{
    if (!this->addr.empty()) {
        Json::Value params;
        params["to"] = routable.get_id().ToString();
        params["amount"] = amount;
        Json::Value val = this->kadc->find_nearest_nodes(params);
        std::cout << val << "\n";
        return std::list<KadNode*>();
    } else {
        return this->find_nearest_nodes_local(routable, amount);
    }
}

/** * Find nodes closest to the given ID.
 *
 * @param routable
 * @param amount
 *
 * @return the list
 */
std::list<KadNode*>
KadNode::find_nearest_nodes_local(KadRoutable routable, int amount)
{
    CBigNum distance = distance_to(routable);
    int bit_length = distance.bit_length();

    int count = 0;
    std::list<KadNode*> closest;

    // First look in the corresponding k-bucket.
    std::list<KadNode*> list = buckets[bit_length];

    list.sort(routable);
    list.unique();

    if (verbose)
        std::cout << "matching k-bucket is " << bit_length << "\n";

    if (!list.empty()) {
        for (std::list<KadNode*>::iterator it = list.begin(); it != list.end();
             ++it) {
            if (count >= amount)
                break;

            if (verbose)
                std::cout << (*it)->get_id().ToString(16) << " distance="
                          << (*it)->distance_to(routable).ToString(16) << "\n";

            closest.push_back(*it);
            count++;
        }
    }

    if (count < amount) {
        std::list<KadNode*> all;

        if (verbose)
            std::cout << "other k-buckets\n";

        // Find remaining nearest nodes.
        for (int i = 1; i < (conf->n_bits + 1); i++) {
            if (bit_length != i) {
                std::list<KadNode*>& list = buckets[i];
                for (std::list<KadNode*>::iterator it = list.begin();
                     it != list.end();
                     ++it) {
                    if (verbose)
                        std::cout << "kbucket " << i << " "
                                  << (*it)->get_id().ToString(16)
                                  << " distance="
                                  << (*it)->distance_to(routable).ToString(16)
                                  << "\n";

                    all.push_back(*it);
                }
            }
        }

        // Sort answers.
        all.sort(routable);
        all.unique();

        for (std::list<KadNode*>::iterator it = all.begin(); it != all.end();
             ++it) {
            if (count >= amount)
                break;

            closest.push_back(*it);
            count++;
        }
    }

    closest.sort(routable);
    closest.unique();

    return closest;
}

/** Print a list of nodes and their distance to target.
 *
 * @param list
 * @param routable
 */
static void print_list(
    std::string comment,
    std::list<KadNode*> list,
    KadRoutable routable,
    std::map<KadNode*, bool>* queried)
{
    std::cout << "---" << comment << " size " << list.size() << "\n";
    std::cout << "target " << routable.get_id().ToString(16) << "\n";
    std::list<KadNode*>::iterator it;
    for (it = list.begin(); it != list.end(); ++it)
        std::cout << "id " << (*it)->get_id().ToString(16) << " eth_account "
                  << (*it)->get_eth_account() << " dist "
                  << (*it)->distance_to(routable).ToString(16) << " queried "
                  << (*queried)[*it] << "\n";
}

/** * Find the node closest to the given ID.
 *
 * @param routable
 *
 * @return the node
 */
std::list<KadNode*> KadNode::lookup(KadRoutable routable)
{
    // Pick our alpha starting nodes.
    std::list<KadNode*> starting_nodes;
    std::list<KadNode*> answers;
    std::map<KadNode*, bool> queried;

    starting_nodes = find_nearest_nodes(routable, conf->alpha);

    if (verbose)
        print_list("starting_nodes", starting_nodes, routable, &queried);

    // Send find_nodes.
    for (std::list<KadNode*>::iterator it = starting_nodes.begin();
         it != starting_nodes.end();
         ++it) {
        std::list<KadNode*> answer =
            (*it)->find_nearest_nodes(routable, conf->k);

        // Add to answers.
        for (std::list<KadNode*>::iterator it2 = answer.begin();
             it2 != answer.end();
             ++it2) {
            // Remove oneself from the list.
            if (get_id() != (*it2)->get_id())
                answers.push_back(*it2);
        }
    }

    // Recursion.
    int round_nb = 0;
    while (true) {
        if (verbose)
            std::cout << "round=" << round_nb << "\n";

        // Sort answers.
        answers.sort(routable);
        answers.unique();

        if (verbose)
            print_list("answers", answers, routable, &queried);

        // Take only k answers.
        std::list<KadNode*> k_answers;
        u_int count = 0;
        for (std::list<KadNode*>::iterator it = answers.begin();
             it != answers.end();
             ++it) {
            if (count >= conf->k)
                break;

            k_answers.push_back(*it);

            count++;
        }

        if (verbose)
            print_list("k_answers", k_answers, routable, &queried);

        std::list<KadNode*> round_answers;

        std::list<KadNode*>::iterator it = k_answers.begin();
        count = 0;
        while (true) {
            if (k_answers.end() == it)
                break;

            if (queried[*it]) {
                it++;
                continue;
            }

            if (count >= conf->alpha)
                break;

            std::list<KadNode*> answer =
                (*it)->find_nearest_nodes(routable, conf->k);

            // Add to round_answers.
            for (std::list<KadNode*>::iterator it2 = answer.begin();
                 it2 != answer.end();
                 ++it2) {
                // Remove oneself from the list.
                if (get_id() != (*it2)->get_id())
                    round_answers.push_back(*it2);
            }

            queried[*it] = true;

            count++;
            it++;
        }

        // Break if we are done with the k_answers and nobody was queried.
        if (k_answers.end() == it && count == 0)
            return k_answers;

        // Sort round answers.
        round_answers.sort(routable);
        round_answers.unique();

        if (verbose)
            print_list("round_answers", round_answers, routable, &queried);

        // Check if found closest.
        bool found_closest;
        if (round_answers.empty() && k_answers.empty())
            found_closest = false;
        else if (!round_answers.empty() && k_answers.empty())
            found_closest = true;
        else if (round_answers.empty() && !k_answers.empty())
            found_closest = false;
        else {
            CBigNum d1 = round_answers.front()->distance_to(routable);
            CBigNum d2 = k_answers.front()->distance_to(routable);
            found_closest = d1 < d2;
        }

        if (verbose)
            std::cout << "found_closest=" << found_closest << "\n";

        if (!found_closest) {
            // Send queries to remaining k nodes.
            while (true) {
                if (k_answers.end() == it)
                    break;

                if (queried[*it]) {
                    it++;
                    continue;
                }

                std::list<KadNode*> answer =
                    (*it)->find_nearest_nodes(routable, conf->k);

                // Add to answers.
                for (std::list<KadNode*>::iterator it2 = answer.begin();
                     it2 != answer.end();
                     ++it2) {
                    // Remove oneself from the list.
                    if (get_id() != (*it2)->get_id())
                        answers.push_back(*it2);
                }

                queried[*it] = true;

                it++;
            }

            round_nb++;
        }
    }

    assert(0);
    return std::list<KadNode*>();
}

void KadNode::store(KadFile* file)
{
    files.push_back(file);
}

std::vector<KadFile*> KadNode::get_files()
{
    return files;
}

void KadNode::show()
{
    std::cout << "id " << get_id().ToString(16) << "\n";
    std::cout << "eth_account " << get_eth_account() << "\n";
    std::cout << "n_conns " << get_n_conns() << "\n";
    save(std::cout);
}

void KadNode::set_verbose(int level)
{
    this->verbose = level;
}

void KadNode::save(std::ostream& fout)
{
    for (int i = 1; i < (conf->n_bits + 1); i++) {
        if (buckets[i].size() > 0) {
            fout << "bucket " << i << "\n";
            std::list<KadNode*>& list = buckets[i];
            for (std::list<KadNode*>::iterator it = list.begin();
                 it != list.end();
                 ++it) {
                fout << (*it)->get_id().ToString(16) << "\n";
            }
        }
    }

    fout << "files\n";
    for (u_int i = 1; i < files.size(); i++) {
        KadFile* file = files[i];
        fout << file->get_id().ToString(16) << "\n";
    }
}

void KadNode::graphviz(std::ostream& fout)
{
    for (int i = 1; i < (conf->n_bits + 1); i++) {
        if (buckets[i].size() > 0) {
            std::list<KadNode*>& list = buckets[i];
            for (std::list<KadNode*>::iterator it = list.begin();
                 it != list.end();
                 ++it)
                fout << "node_" << get_id().ToString(16) << " -> node_"
                     << (*it)->get_id().ToString(16) << ";\n";
        }
    }
}

// TODO: factorize all of this
void KadNode::buy_storage(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyStorage(address,address,uint256)").substr(0, 8)
    const std::string selector = "0x366e4d";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        call_contract(conf->geth, eth_account, QUADIRON_CONTRACT_ADDR, payload);
    } catch (jsonrpc::JsonRpcException exn) {
        std::cerr << "cannot buy " << nb_bytes << "bytes from " << seller
                  << ": " << exn.what() << '\n';
    }
}

void KadNode::put_bytes(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyerPutBytes(address,address,uint256)").substr(0, 8)
    const std::string selector = "0xed69a0";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        call_contract(conf->geth, eth_account, QUADIRON_CONTRACT_ADDR, payload);
    } catch (jsonrpc::JsonRpcException exn) {
        std::cerr << "cannot put " << nb_bytes << "bytes on storage of "
                  << seller << ": " << exn.what() << '\n';
    }
}

void KadNode::get_bytes(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyerGetBytes(address,address,uint256)").substr(0, 8)
    const std::string selector = "0xdad071";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        call_contract(conf->geth, eth_account, QUADIRON_CONTRACT_ADDR, payload);
    } catch (jsonrpc::JsonRpcException exn) {
        std::cerr << "cannot get " << nb_bytes << "bytes from storage of "
                  << seller << ": " << exn.what() << '\n';
    }
}
