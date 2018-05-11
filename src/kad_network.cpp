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
#include "dht/dht.h"
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

        const UInt160 id(bitmap.get_rand_uint() * keyspace);
        std::string ip("127.0.0.1");
        NodeLocalCom node_com(this);

        // Create remote node from a bootstrap.
        if (!bstraplist.empty()) {
            ip = bstraplist.back();
            std::cout << "create remote node (" << ip << ")\n";
        }
        const dht::NodeAddress addr{id, ip, 0};
        auto node = std::make_unique<Node<NodeLocalCom>>(*conf, addr, node_com);
        nodes_map[node->id().to_string()] = node.get();
        nodes.push_back(std::move(node));
    }

    // There shall be a responsable for every portion of the keyspace.
    assert(bitmap.is_exhausted());

    // Continue creating conns for the nodes that dont meet the initial number
    // required.
    std::uniform_int_distribution<uint64_t> dis(0, nodes.size() - 1);
    for (uint32_t i = 0; i < conf->n_nodes; i++) {
        std::unique_ptr<Node<NodeLocalCom>>& node = nodes[i];

        if ((i % 1000) == 0) {
            std::cerr << "node " << i << "/" << conf->n_nodes
                      << "                   \r";
        }

        uint32_t guard = 0;
        while (node->connection_count() < n_initial_conn) {
            if (guard >= (2 * conf->n_nodes)) {
                std::cout << "forgiving required conditions for " << node->id()
                          << ", it has only " << node->connection_count()
                          << " connections\n";
                break;
            }

            // Pick a random node.
            std::unique_ptr<Node<NodeLocalCom>>& other = nodes[dis(prng())];
            if (*node == *other) {
                continue;
            }

            // Connect them 2-way.
            node->ping(*other);
            other->ping(*node);

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
        std::unique_ptr<Node<NodeLocalCom>>& node = nodes[dis(prng())];

        // Generate a random key for the file.
        const UInt160 key(UInt160::rand(prng(), conf->n_bits));
        node->store(std::make_unique<File>(key, ""));
        files.push_back(key);

        // Store file at multiple location.
        for (auto& it : node->node_lookup(key)) {
            const auto dst = lookup_cheat(it.id().to_string());
            dst->store(std::make_unique<File>(key, ""));
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
    for (auto& file_key : files) {
        if ((n_files % 1000) == 0) {
            std::cerr << "file " << n_files << "/" << files.size()
                      << "                   \r";
        }

        // Take a random node.
        std::unique_ptr<Node<NodeLocalCom>>& node = nodes[dis(prng())];

        // Check that at least one node has the file.
        bool found = false;
        for (const auto& it : node->node_lookup(file_key)) {
            const auto kad_node = lookup_cheat(it.id().to_string());

            for (const auto& node_file_key : kad_node->files()) {
                if (node_file_key == file_key) {
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            std::cerr << "file " << file_key << " was not found\n";
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

void Network::rand_key(tkey_callback_func cb_func, void* cb_arg)
{
    if (nullptr != cb_func) {
        cb_func(UInt160::rand(prng(), conf->n_bits), cb_arg);
    }
}

/** Lookup a node by its id
 *
 * @param id node ID
 *
 * @return the node identified by `id`
 */
Node<NodeLocalCom>* Network::lookup_cheat(const std::string& id) const
{
    return nodes_map.at(id);
}

/** Find node nearest to specified routable. */
Node<NodeLocalCom>* Network::find_nearest_cheat(const UInt160& target_id)
{
    const auto nearest = std::min_element(
        nodes.begin(),
        nodes.end(),
        [&target_id](
            std::unique_ptr<Node<NodeLocalCom>>& n1,
            std::unique_ptr<Node<NodeLocalCom>>& n2) {
            const UInt160 d1(n1->distance_to(target_id));
            const UInt160 d2(n2->distance_to(target_id));
            return d1 < d2;
        });
    return nearest != nodes.end() ? nearest->get() : nullptr;
}

void Network::save(std::ostream& fout)
{
    conf->save(fout);
    for (uint32_t i = 0; i < conf->n_nodes; i++) {
        fout << "node " << i << " " << nodes[i]->id() << "\n";
        nodes[i]->save(fout);
    }
}

void Network::graphviz(std::ostream& fout)
{
    fout << "digraph G {\n";
    fout << "  node [shape=record];\n";
    fout << "  rankdir=TB;\n";
    for (uint32_t i = 0; i < conf->n_nodes; i++) {
        fout << "node_" << nodes[i]->id() << " [color=blue, label=\""
             << nodes[i]->id() << "\"];\n";
        nodes[i]->graphviz(fout);
    }
    fout << "}\n";
}

} // namespace kad
