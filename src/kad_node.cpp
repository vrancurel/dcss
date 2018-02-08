#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "bignum.h"
#include "kad_conf.h"
#include "kad_file.h"
#include "kad_node.h"
#include "nodeclient.h"

namespace kad {

// Address of the QuadIron contract on the blockchain.
#define QUADIRON_CONTRACT_ADDR "0x5e667a8D97fBDb2D3923a55b295DcB8f5985FB79"

static void call_contract(
    GethClient& geth,
    const std::string& node_addr,
    const std::string& contract_addr,
    const std::string& payload)
{
    Json::Value params;

    params["from"] = node_addr;
    params["to"] = contract_addr;
    params["data"] = payload;

    const std::string tx_hash = geth.eth_sendTransaction(params);
    std::cout << "tx_hash: " << tx_hash << '\n';

    // FIXME: busy way is ugly.
    while (true) {
        try {
            const Json::Value receipt = geth.eth_getTransactionReceipt(tx_hash);
            std::cout << "result: " << receipt.toStyledString() << '\n';
            // TODO: we should probably return a bool to the caller, or raise…
            if (receipt["status"] == "0x0") {
                std::cout << "transaction failed\n";
            } else {
                std::cout << "transaction successed: " << receipt["status"]
                          << '\n';
            }
            return;
        } catch (jsonrpc::JsonRpcException& exn) {
            if (exn.GetCode() == -32000) {
                continue; // Transaction is pending…
            }
            fprintf(stderr, "error: %s\n", exn.what());
            throw;
        }
    }
}

// Encode an integer as an uint256 according to the Ethereum Contract ABI.
// See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI:
//
// > uint<M>: enc(X) is the big-endian encoding of X, padded on the
// > higher-order (left) side with zero-bytes such that the length is a
// > multiple of 32 bytes.
static inline std::string encode_uint256(uint64_t v)
{
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(64) << std::hex << v;
    return oss.str();
}

// Address are encoded as uint160.
static inline std::string encode_address(const std::string& addr)
{
    std::ostringstream oss;
    // Skip the leading 0x, pad for 160 bytes.
    oss << std::setfill('0') << std::setw(40) << addr.substr(2);
    return oss.str();
}

Node::Node(
    const Conf& configuration,
    const CBigNum& node_id,
    const std::string& rpc_addr)
    : Routable(node_id, KAD_ROUTABLE_NODE), conf(&configuration)
{
    this->verbose = false;
    this->addr = rpc_addr;
    if (!addr.empty()) {
        // FIXME use port
        this->httpclient = new jsonrpc::HttpClient(addr);
        this->nodec = new NodeClient(*this->httpclient);
    }

    // The passphrase of the account is the hex of the node ID.
    this->eth_passphrase = id.ToString(16);
    try {
        this->eth_account = conf->geth.personal_newAccount(eth_passphrase);
        conf->geth.personal_unlockAccount(eth_account, eth_passphrase, 0);
    } catch (jsonrpc::JsonRpcException&) {
        this->eth_account = "";
    }
}

Node::Node(const Conf& configuration, const CBigNum& node_id)
    : Node(configuration, node_id, "")
{
}

const std::string& Node::get_eth_account() const
{
    return eth_account;
}

/** Get the number of conns. */
uint32_t Node::get_n_conns()
{
    uint32_t total;

    total = 0;
    for (uint32_t i = 1; i < (conf->n_bits + 1); i++) {
        total += static_cast<uint32_t>(buckets[i].size());
    }

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
bool Node::add_conn(Node* node, bool contacted_us)
{
    if (node->get_id() == get_id()) {
        std::cout << "cannot add itself " << get_id().ToString(16) << std::endl;
        return false;
    }

    CBigNum distance = distance_to(*node);
    uint32_t bit_length = distance.bit_length();

    std::list<Node*>& list = buckets[bit_length];
    std::list<Node*>::iterator it;
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
        }
        // Nothing to do.
        return false;
    }

    // Add node in bucket.
    if (buckets[bit_length].size() < conf->k) {
        buckets[bit_length].push_back(node);
        return true;
    }

    return false;
}

std::list<Node*>
Node::find_nearest_nodes(const Routable& routable, uint32_t amount)
{
    if (!this->addr.empty()) {
        Json::Value params;
        params["to"] = routable.get_id().ToString();
        params["amount"] = amount;
        Json::Value val = this->nodec->find_nearest_nodes(params);
        std::cout << val << "\n";
        return std::list<Node*>();
    }
    return this->find_nearest_nodes_local(routable, amount);
}

/** * Find nodes closest to the given ID. */
std::list<Node*>
Node::find_nearest_nodes_local(const Routable& routable, uint32_t amount)
{
    CBigNum distance = distance_to(routable);
    uint32_t bit_length = distance.bit_length();

    uint32_t count = 0;
    std::list<Node*> closest;

    // First look in the corresponding k-bucket.
    std::list<Node*> list = buckets[bit_length];

    list.sort(routable);
    list.unique();

    if (verbose) {
        std::cout << "matching k-bucket is " << bit_length << "\n";
    }

    if (!list.empty()) {
        for (auto& it : list) {
            if (count >= amount) {
                break;
            }

            if (verbose) {
                std::cout << it->get_id().ToString(16) << " distance="
                          << it->distance_to(routable).ToString(16) << "\n";
            }

            closest.push_back(it);
            count++;
        }
    }

    if (count < amount) {
        std::list<Node*> all;

        if (verbose) {
            std::cout << "other k-buckets\n";
        }

        // Find remaining nearest nodes.
        for (uint32_t i = 1; i < (conf->n_bits + 1); i++) {
            if (bit_length != i) {
                std::list<Node*>& nodes = buckets[i];
                for (auto& it : nodes) {
                    if (verbose) {
                        std::cout << "kbucket " << i << " "
                                  << it->get_id().ToString(16) << " distance="
                                  << it->distance_to(routable).ToString(16)
                                  << "\n";
                    }

                    all.push_back(it);
                }
            }
        }

        // Sort answers.
        all.sort(routable);
        all.unique();

        for (auto& it : all) {
            if (count >= amount) {
                break;
            }

            closest.push_back(it);
            count++;
        }
    }

    closest.sort(routable);
    closest.unique();

    return closest;
}

/** Print a list of nodes and their distance to target. */
static void print_list(
    const std::string& comment,
    std::list<Node*> list,
    const Routable& routable,
    std::map<Node*, bool>* queried)
{
    std::cout << "---" << comment << " size " << list.size() << "\n";
    std::cout << "target " << routable.get_id().ToString(16) << "\n";
    std::list<Node*>::iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        std::cout << "id " << (*it)->get_id().ToString(16) << " eth_account "
                  << (*it)->get_eth_account() << " dist "
                  << (*it)->distance_to(routable).ToString(16) << " queried "
                  << (*queried)[*it] << "\n";
    }
}

/** * Find the node closest to the given ID. */
std::list<Node*> Node::lookup(const Routable& routable)
{
    // Pick our alpha starting nodes.
    std::list<Node*> starting_nodes;
    std::list<Node*> answers;
    std::map<Node*, bool> queried;

    starting_nodes = find_nearest_nodes(routable, conf->alpha);

    if (verbose) {
        print_list("starting_nodes", starting_nodes, routable, &queried);
    }

    // Send find_nodes.
    for (auto& starting_node : starting_nodes) {
        std::list<Node*> nodes =
            starting_node->find_nearest_nodes(routable, conf->k);

        // Add to answers.
        for (auto& answer : nodes) {
            // Remove oneself from the list.
            if (get_id() != answer->get_id()) {
                answers.push_back(answer);
            }
        }
    }

    // Recursion.
    int round_nb = 0;
    while (true) {
        if (verbose) {
            std::cout << "round=" << round_nb << "\n";
        }

        // Sort answers.
        answers.sort(routable);
        answers.unique();

        if (verbose) {
            print_list("answers", answers, routable, &queried);
        }

        // Take only k answers.
        std::list<Node*> k_answers;
        uint32_t count = 0;
        for (auto& answer : answers) {
            if (count >= conf->k) {
                break;
            }

            k_answers.push_back(answer);

            count++;
        }

        if (verbose) {
            print_list("k_answers", k_answers, routable, &queried);
        }

        std::list<Node*> round_answers;

        auto it = k_answers.begin();
        count = 0;
        while (true) {
            if (k_answers.end() == it) {
                break;
            }

            if (queried[*it]) {
                it++;
                continue;
            }

            if (count >= conf->alpha) {
                break;
            }

            std::list<Node*> nodes =
                (*it)->find_nearest_nodes(routable, conf->k);

            // Add to round_answers.
            for (auto& answer : nodes) {
                // Remove oneself from the list.
                if (get_id() != answer->get_id()) {
                    round_answers.push_back(answer);
                }
            }

            queried[*it] = true;

            count++;
            it++;
        }

        // Break if we are done with the k_answers and nobody was queried.
        if (k_answers.end() == it && count == 0) {
            return k_answers;
        }

        // Sort round answers.
        round_answers.sort(routable);
        round_answers.unique();

        if (verbose) {
            print_list("round_answers", round_answers, routable, &queried);
        }

        // Check if found closest.
        bool found_closest;
        if (round_answers.empty() && k_answers.empty()) {
            found_closest = false;
        } else if (!round_answers.empty() && k_answers.empty()) {
            found_closest = true;
        } else if (round_answers.empty() && !k_answers.empty()) {
            found_closest = false;
        } else {
            CBigNum d1 = round_answers.front()->distance_to(routable);
            CBigNum d2 = k_answers.front()->distance_to(routable);
            found_closest = d1 < d2;
        }

        if (verbose) {
            std::cout << "found_closest=" << found_closest << "\n";
        }

        if (!found_closest) {
            // Send queries to remaining k nodes.
            while (true) {
                if (k_answers.end() == it) {
                    break;
                }

                if (queried[*it]) {
                    it++;
                    continue;
                }

                std::list<Node*> nodes =
                    (*it)->find_nearest_nodes(routable, conf->k);

                // Add to answers.
                for (auto& answer : nodes) {
                    // Remove oneself from the list.
                    if (get_id() != answer->get_id()) {
                        answers.push_back(answer);
                    }
                }

                queried[*it] = true;

                it++;
            }

            round_nb++;
        }
    }

    assert(0);
    return std::list<Node*>();
}

void Node::store(File* file)
{
    files.push_back(file);
}

std::vector<File*> Node::get_files()
{
    return files;
}

void Node::show()
{
    std::cout << "id " << get_id().ToString(16) << "\n";
    std::cout << "eth_account " << get_eth_account() << "\n";
    std::cout << "n_conns " << get_n_conns() << "\n";
    save(std::cout);
}

void Node::set_verbose(bool enable)
{
    this->verbose = enable;
}

void Node::save(std::ostream& fout)
{
    for (uint32_t i = 1; i < (conf->n_bits + 1); i++) {
        if (!buckets[i].empty()) {
            fout << "bucket " << i << "\n";
            std::list<Node*>& list = buckets[i];
            for (auto& it : list) {
                fout << it->get_id().ToString(16) << "\n";
            }
        }
    }

    fout << "files\n";
    for (std::vector<File*>::size_type i = 1; i < files.size(); i++) {
        File* file = files[i];
        fout << file->get_id().ToString(16) << "\n";
    }
}

void Node::graphviz(std::ostream& fout)
{
    for (uint32_t i = 1; i < (conf->n_bits + 1); i++) {
        if (!buckets[i].empty()) {
            std::list<Node*>& list = buckets[i];
            for (auto& it : list) {
                fout << "node_" << get_id().ToString(16) << " -> node_"
                     << it->get_id().ToString(16) << ";\n";
            }
        }
    }
}

// TODO: factorize all of this
void Node::buy_storage(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyStorage(address,address,uint256)").substr(0, 8)
    const std::string selector = "0x366e4d";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        call_contract(conf->geth, eth_account, QUADIRON_CONTRACT_ADDR, payload);
    } catch (jsonrpc::JsonRpcException& exn) {
        std::cerr << "cannot buy " << nb_bytes << "bytes from " << seller
                  << ": " << exn.what() << '\n';
    }
}

void Node::put_bytes(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyerPutBytes(address,address,uint256)").substr(0, 8)
    const std::string selector = "0xed69a0";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        call_contract(conf->geth, eth_account, QUADIRON_CONTRACT_ADDR, payload);
    } catch (jsonrpc::JsonRpcException& exn) {
        std::cerr << "cannot put " << nb_bytes << "bytes on storage of "
                  << seller << ": " << exn.what() << '\n';
    }
}

void Node::get_bytes(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyerGetBytes(address,address,uint256)").substr(0, 8)
    const std::string selector = "0xdad071";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        call_contract(conf->geth, eth_account, QUADIRON_CONTRACT_ADDR, payload);
    } catch (jsonrpc::JsonRpcException& exn) {
        std::cerr << "cannot get " << nb_bytes << "bytes from storage of "
                  << seller << ": " << exn.what() << '\n';
    }
}

} // namespace kad
