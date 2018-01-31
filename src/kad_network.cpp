#include <cassert>
#include <cstdint>
#include <iostream>

#include "bit_map.h"
#include "kad_conf.h"
#include "kad_file.h"
#include "kad_network.h"
#include "kad_node.h"
#include "utils.h"

KadNetwork::KadNetwork(KadConf* conf)
{
    this->conf = conf;
}

/** Initialize nodes. */
void KadNetwork::initialize_nodes(
    int n_initial_conn,
    std::vector<std::string> bstraplist)
{
    std::cout << "initialize nodes\n";

    BitMap bitmap(conf->n_nodes);

    // Split the total keyspace equally among nodes.
    CBigNum keyspace = CBigNum(1);
    keyspace = (keyspace << conf->n_bits);
    keyspace /= conf->n_nodes;

    // Create nodes.
    for (u_int i = 0; i < conf->n_nodes; i++) {
        std::cerr << "creating node " << i << '\n';
        KadNode* node;
        if (!bstraplist.empty()) {
            std::string bstrap = bstraplist.back();
            bstraplist.pop_back();
            std::cout << "create remote node (" << bstrap << ")\n";
            // Create remote node from a bootstrap.
            node = new KadNode(conf, bitmap.get_rand_uint() * keyspace, bstrap);
        } else {
            // Simulate node.
            node = new KadNode(conf, bitmap.get_rand_uint() * keyspace);
        }
        nodes.push_back(node);
        nodes_map[node->get_id().ToString(16)] = node;
    }

    // There shall be a responsable for every portion of the keyspace.
    assert(bitmap.is_exhausted());

    // Continue creating conns for the nodes that dont meet the initial number
    // required.
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);
    for (u_int i = 0; i < conf->n_nodes; i++) {
        KadNode* node = nodes[i];

        if ((i % 1000) == 0) {
            std::cerr << "node " << i << "/" << conf->n_nodes
                      << "                   \r";
        }

        u_int guard = 0;
        while (node->get_n_conns() < n_initial_conn) {
            KadNode* other;

            if (guard >= (2 * conf->n_nodes)) {
                std::cout << "forgiving required conditions for "
                          << node->get_id().ToString(16) << ", it has only "
                          << node->get_n_conns() << " connections\n";
                break;
            }

            // Pick a random node.
            other = nodes[dis(prng())];

            // Connect them 2-way.
            node->add_conn(other, false);
            other->add_conn(node, false);

            guard++;
        }
    }

    std::cout << "\n";
}

void KadNetwork::initialize_files(int n_files)
{
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);

    std::cout << "initialize files\n";

    for (int i = 0; i < n_files; i++) {
        if ((i % 1000) == 0) {
            std::cerr << "file " << i << "/" << n_files
                      << "                   \r";
        }

        // Take a random node.
        KadNode* node = nodes[dis(prng())];

        // Gen a random identifier for the file.
        CBigNum bn;
        bn.Rand(conf->n_bits);
        auto* file = new KadFile(bn, node);
        files.push_back(file);

        // Store file at multiple location.
        std::list<KadNode*> result = node->lookup(*file);
        for (auto& it : result) {
            it->store(file);
        }
    }
}

/** Check that files are accessible from random nodes. */
void KadNetwork::check_files()
{
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);

    std::cout << "checking files\n";

    int n_wrong = 0;
    for (u_int i = 0; i < files.size(); i++) {
        if ((i % 1000) == 0) {
            std::cerr << "file " << i << "/" << files.size()
                      << "                   \r";
        }

        KadFile* file = files[i];

        // Take a random node.
        KadNode* node = nodes[dis(prng())];

        // Get results.
        std::list<KadNode*> result = node->lookup(*file);

        // Check that at least one result has the file.
        bool found = false;
        for (auto& it : result) {
            std::vector<KadFile*> node_files = it->get_files();
            for (auto node_file : node_files) {
                if (node_file == file) {
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            std::cerr << "file " << file->get_id().ToString(16)
                      << " who was referenced by "
                      << file->get_referencer()->get_id().ToString(16)
                      << " was not found\n";
            n_wrong++;
        }
    }
    std::cerr << n_wrong << "/" << files.size() << " files wrongly stored\n";
}

void KadNetwork::rand_node(tnode_callback_func cb_func, void* cb_arg)
{
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);
    uint64_t x = dis(prng());
    if (nullptr != cb_func) {
        cb_func(nodes[x], cb_arg);
    }
}

void KadNetwork::rand_routable(troutable_callback_func cb_func, void* cb_arg)
{
    CBigNum bn;
    bn.Rand(conf->n_bits);
    KadRoutable routable(bn, KAD_ROUTABLE_FILE);
    if (nullptr != cb_func) {
        cb_func(routable, cb_arg);
    }
}

/** Lookup a node by its id
 *
 * @param id node IS
 *
 * @return the node identified by `id`
 */
KadNode* KadNetwork::lookup_cheat(const std::string& id)
{
    return nodes_map[id];
}

/** Find node nearest to specified routable. */
KadNode* KadNetwork::find_nearest_cheat(const KadRoutable& routable)
{
    KadNode* nearest = nullptr;

    for (auto& node : nodes) {
        if (nullptr == nearest) {
            nearest = node;
        } else {
            CBigNum d1 = node->distance_to(routable);
            CBigNum d2 = nearest->distance_to(routable);

            if (d1 < d2) {
                nearest = node;
            }
        }
    }

    return nearest;
}

void KadNetwork::save(std::ostream& fout)
{
    conf->save(fout);
    for (u_int i = 0; i < conf->n_nodes; i++) {
        KadNode* node = nodes[i];

        fout << "node " << i << " " << node->get_id().ToString(16) << "\n";
        node->save(fout);
    }
}

void KadNetwork::graphviz(std::ostream& fout)
{
    fout << "digraph G {\n";
    fout << "  node [shape=record];\n";
    fout << "  rankdir=TB;\n";

    for (u_int i = 0; i < conf->n_nodes; i++) {
        KadNode* node = nodes[i];

        fout << "node_" << node->get_id().ToString(16)
             << " [color=blue, label=\"" << node->get_id().ToString(16)
             << "\"];\n";
        node->graphviz(fout);
    }

    fout << "}\n";
}
