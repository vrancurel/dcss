/*
 * Copyright 2017-2018 the QuadIron authors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <cassert>
#include <cstdint>
#include <iostream>

#include "bit_map.h"
#include "kad_conf.h"
#include "kad_file.h"
#include "kad_network.h"
#include "kad_node.h"
#include "utils.h"

namespace kad {

Network::Network(const Conf& configuration) : conf(&configuration) {}

/** Initialize nodes. */
void Network::initialize_nodes(
    uint32_t n_initial_conn,
    std::vector<std::string> bstraplist)
{
    std::cout << "initialize nodes\n";

    BitMap bitmap(conf->n_nodes);

    // Split the total keyspace equally among nodes.
    UInt160 keyspace(1u);
    keyspace = (keyspace << conf->n_bits);
    keyspace /= conf->n_nodes;

    // Create nodes.
    for (uint32_t i = 0; i < conf->n_nodes; i++) {
        std::cerr << "creating node " << i << '\n';
        Node* node;
        if (!bstraplist.empty()) {
            std::string bstrap = bstraplist.back();
            bstraplist.pop_back();
            std::cout << "create remote node (" << bstrap << ")\n";
            // Create remote node from a bootstrap.
            node = new Node(*conf, bitmap.get_rand_uint() * keyspace, bstrap);
        } else {
            // Simulate node.
            node = new Node(*conf, bitmap.get_rand_uint() * keyspace);
        }
        nodes.push_back(node);
        nodes_map[node->get_id().to_string()] = node;
    }

    // There shall be a responsable for every portion of the keyspace.
    assert(bitmap.is_exhausted());

    // Continue creating conns for the nodes that dont meet the initial number
    // required.
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);
    for (uint32_t i = 0; i < conf->n_nodes; i++) {
        Node* node = nodes[i];

        if ((i % 1000) == 0) {
            std::cerr << "node " << i << "/" << conf->n_nodes
                      << "                   \r";
        }

        uint32_t guard = 0;
        while (node->get_n_conns() < n_initial_conn) {
            Node* other;

            if (guard >= (2 * conf->n_nodes)) {
                std::cout << "forgiving required conditions for "
                          << node->get_id() << ", it has only "
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

void Network::initialize_files(uint32_t n_files)
{
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);

    std::cout << "initialize files\n";

    for (uint32_t i = 0; i < n_files; i++) {
        if ((i % 1000) == 0) {
            std::cerr << "file " << i << "/" << n_files
                      << "                   \r";
        }

        // Take a random node.
        Node* node = nodes[dis(prng())];

        // Generate a random key for the file.
        const UInt160 key(UInt160::rand(prng(), conf->n_bits));
        auto* file = new File(key, "");
        files.push_back(file);

        // Store file at multiple location.
        std::list<Node*> result = node->lookup(*file);
        for (auto& it : result) {
            it->store(file);
        }
    }
}

/** Check that files are accessible from random nodes. */
void Network::check_files()
{
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);

    std::cout << "checking files\n";

    uint64_t n_wrong = 0;
    uint64_t n_files = 0;
    for (auto& file : files) {
        if ((n_files % 1000) == 0) {
            std::cerr << "file " << n_files << "/" << files.size()
                      << "                   \r";
        }

        // Take a random node.
        Node* node = nodes[dis(prng())];

        // Get results.
        std::list<Node*> result = node->lookup(*file);

        // Check that at least one result has the file.
        bool found = false;
        for (auto& it : result) {
            std::vector<File*> node_files = it->get_files();
            for (auto node_file : node_files) {
                if (node_file == file) {
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            std::cerr << "file " << file->key() << " was not found\n";
            n_wrong++;
        }
        ++n_files;
    }
    std::cerr << n_wrong << "/" << files.size() << " files wrongly stored\n";
}

void Network::rand_node(tnode_callback_func cb_func, void* cb_arg)
{
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);
    uint64_t x = dis(prng());
    if (nullptr != cb_func) {
        cb_func(*nodes[x], cb_arg);
    }
}

void Network::rand_routable(troutable_callback_func cb_func, void* cb_arg)
{
    const UInt160 bn(UInt160::rand(prng(), conf->n_bits));
    Routable routable(bn, KAD_ROUTABLE_FILE);
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
Node* Network::lookup_cheat(const std::string& id)
{
    return nodes_map[id];
}

/** Find node nearest to specified routable. */
Node* Network::find_nearest_cheat(const Routable& routable)
{
    Node* nearest = nullptr;

    for (auto& node : nodes) {
        if (nullptr == nearest) {
            nearest = node;
        } else {
            const UInt160 d1(node->distance_to(routable));
            const UInt160 d2(nearest->distance_to(routable));

            if (d1 < d2) {
                nearest = node;
            }
        }
    }

    return nearest;
}

void Network::save(std::ostream& fout)
{
    conf->save(fout);
    for (uint32_t i = 0; i < conf->n_nodes; i++) {
        Node* node = nodes[i];

        fout << "node " << i << " " << node->get_id() << "\n";
        node->save(fout);
    }
}

void Network::graphviz(std::ostream& fout)
{
    fout << "digraph G {\n";
    fout << "  node [shape=record];\n";
    fout << "  rankdir=TB;\n";

    for (uint32_t i = 0; i < conf->n_nodes; i++) {
        Node* node = nodes[i];

        fout << "node_" << node->get_id() << " [color=blue, label=\""
             << node->get_id() << "\"];\n";
        node->graphviz(fout);
    }

    fout << "}\n";
}

} // namespace kad
