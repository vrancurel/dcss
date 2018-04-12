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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>

#include "cmds.h"
#include "dht/dht.h"
#include "kad_network.h"
#include "kad_node.h"
#include "kad_node_com.h"
#include "kad_routable.h"
#include "shell.h"
#include "uint160.h"
#include "utils.h"

namespace kad {

static int cmd_quit(Shell* /*shell*/, int /*argc*/, char** /*argv*/)
{
    return SHELL_RETURN;
}

static int cmd_help(Shell* /*shell*/, int argc, char** argv)
{
    if (argc == 1) {
        struct cmd_def* cmdp;
        int j = 0;

        for (int i = 0; cmd_defs[i] != nullptr; ++i) {
            cmdp = cmd_defs[i];
            std::cout << std::setw(16) << cmdp->name;
            j++;
            if (j == 4) {
                std::cout << '\n';
                j = 0;
            }
        }
        std::cout << '\n';
    } else if (argc == 2) {
        struct cmd_def* cmdp;

        for (int i = 0; cmd_defs[i] != nullptr; ++i) {
            cmdp = cmd_defs[i];
            if (strcmp(argv[1], cmdp->name) == 0) {
                std::cout << cmdp->purpose << '\n';
                break;
            }
        }
    }

    return SHELL_CONT;
}

static void cb_display_node(const Node<NodeLocalCom>& node, void* /*cb_arg*/)
{
    std::cout << node.id() << '\n';
}

static void cb_display_key(const UInt160& key, void* /*cb_arg*/)
{
    std::cout << key << '\n';
}

static int cmd_rand_node(Shell* shell, int /*argc*/, char** /*argv*/)
{
    auto* network = static_cast<Network*>(shell->get_handle());

    network->rand_node(cb_display_node, nullptr);

    return SHELL_CONT;
}

static int cmd_rand_key(Shell* shell, int /*argc*/, char** /*argv*/)
{
    auto* network = static_cast<Network*>(shell->get_handle());

    network->rand_key(cb_display_key, nullptr);

    return SHELL_CONT;
}

static int cmd_jump(Shell* shell, int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: jump key\n";
        return SHELL_CONT;
    }

    auto* network = static_cast<Network*>(shell->get_handle());

    Node<NodeLocalCom>* node = network->lookup_cheat(argv[1]);
    if (nullptr == node) {
        std::cerr << "not found\n";
        return SHELL_CONT;
    }

    shell->set_handle2(node);

    return SHELL_CONT;
}

static int cmd_lookup(Shell* shell, int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: lookup key\n";
        return SHELL_CONT;
    }

    auto* start_node = static_cast<Node<NodeLocalCom>*>(shell->get_handle2());
    if (start_node == nullptr) {
        std::cerr << "shall jump to a node first\n";
        return SHELL_CONT;
    }

    const UInt160 node_id(argv[1]);
    for (const auto& node : start_node->find_node(node_id, stou32(argv[2]))) {
        std::cout << "id " << node.id() << " dist "
                  << dht::compute_distance(node.id(), node_id) << "\n";
    }

    return SHELL_CONT;
}

static int cmd_find_nearest(Shell* shell, int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "usage: find_nearest node_id nb_node\n";
        return SHELL_CONT;
    }

    auto* start_node = static_cast<Node<NodeLocalCom>*>(shell->get_handle2());
    if (start_node == nullptr) {
        std::cerr << "shall jump to a node first\n";
        return SHELL_CONT;
    }

    const UInt160 node_id(argv[1]);
    for (const auto& node : start_node->find_node(node_id, stou32(argv[2]))) {
        std::cout << "id " << node.id() << " dist "
                  << dht::compute_distance(node.id(), node_id) << "\n";
    }

    return SHELL_CONT;
}

static int cmd_show(Shell* shell, int argc, char** /*argv*/)
{
    if (argc != 1) {
        std::cerr << "usage: show\n";
        return SHELL_CONT;
    }

    auto* node = static_cast<Node<NodeLocalCom>*>(shell->get_handle2());

    if (nullptr == node) {
        std::cerr << "shall jump to a node first\n";
        return SHELL_CONT;
    }

    node->show();

    return SHELL_CONT;
}

static int cmd_verbose(Shell* shell, int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: verbose 1|0\n";
        return SHELL_CONT;
    }

    auto* node = static_cast<Node<NodeLocalCom>*>(shell->get_handle2());

    if (nullptr == node) {
        std::cerr << "shall jump to a node first\n";
        return SHELL_CONT;
    }

    node->set_verbose(std::stoi(argv[1]) != 0);

    return SHELL_CONT;
}

static int cmd_cheat_lookup(Shell* shell, int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: jump node_id\n";
        return SHELL_CONT;
    }

    auto* network = static_cast<Network*>(shell->get_handle());

    const UInt160 node_id(argv[1]);
    Node<NodeLocalCom>* node = network->find_nearest_cheat(node_id);
    if (nullptr == node) {
        std::cerr << "not found\n";
        return SHELL_CONT;
    }

    cb_display_node(*node, nullptr);

    return SHELL_CONT;
}

static int cmd_save(Shell* shell, int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: save file\n";
        return SHELL_CONT;
    }

    auto* network = static_cast<Network*>(shell->get_handle());

    std::ofstream fout(argv[1]);
    network->save(fout);

    return SHELL_CONT;
}

static int cmd_distance(Shell* /*shell*/, int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "usage: distance id1 id2\n";
        return SHELL_CONT;
    }

    const UInt160 id1(argv[1]);
    const UInt160 id2(argv[2]);

    std::cout << dht::compute_distance(id1, id2) << "\n";

    return SHELL_CONT;
}

static int cmd_bit_length(Shell* /*shell*/, int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: bit_length bn\n";
        return SHELL_CONT;
    }

    const UInt160 bn(argv[1]);

    std::cout << bn.bit_length() << "\n";

    return SHELL_CONT;
}

static int cmd_graphviz(Shell* shell, int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: graphviz file\n";
        return SHELL_CONT;
    }

    auto* network = static_cast<Network*>(shell->get_handle());

    std::ofstream fout(argv[1]);
    network->graphviz(fout);

    return SHELL_CONT;
}

static int cmd_buy_storage(Shell* shell, int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "usage: buy_storage SELLER N_BYTES\n";
        return SHELL_CONT;
    }

    auto* node = static_cast<Node<NodeLocalCom>*>(shell->get_handle2());
    if (nullptr == node) {
        std::cerr << "shall jump to a node first\n";
        return SHELL_CONT;
    }

    node->buy_storage(argv[1], stou64(argv[2]));

    return SHELL_CONT;
}

static int cmd_put_bytes(Shell* shell, int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "usage: put_bytes SELLER N_BYTES\n";
        return SHELL_CONT;
    }

    auto* node = static_cast<Node<NodeLocalCom>*>(shell->get_handle2());
    if (nullptr == node) {
        std::cerr << "shall jump to a node first\n";
        return SHELL_CONT;
    }

    node->put_bytes(argv[1], stou64(argv[2]));

    return SHELL_CONT;
}

static int cmd_get_bytes(Shell* shell, int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "usage: get_bytes SELLER N_BYTES\n";
        return SHELL_CONT;
    }

    auto* node = static_cast<Node<NodeLocalCom>*>(shell->get_handle2());
    if (nullptr == node) {
        std::cerr << "shall jump to a node first\n";
        return SHELL_CONT;
    }

    node->get_bytes(argv[1], stou64(argv[2]));

    return SHELL_CONT;
}

struct cmd_def quit_cmd = {"quit", "quit program", cmd_quit};
struct cmd_def help_cmd = {"help", "help", cmd_help};
struct cmd_def jump_cmd = {"jump", "jump to a node", cmd_jump};
struct cmd_def lookup_cmd = {"lookup", "lookup a node", cmd_lookup};
struct cmd_def cheat_lookup_cmd = {"cheat_lookup",
                                   "lookup the closest node by cheating",
                                   cmd_cheat_lookup};
struct cmd_def rand_node_cmd = {"rand_node",
                                "print a random node id",
                                cmd_rand_node};
struct cmd_def rand_key_cmd = {"rand_key", "print a random key", cmd_rand_key};
struct cmd_def show_cmd = {"show", "show k-buckets", cmd_show};
struct cmd_def find_nearest_cmd = {"find_nearest",
                                   "find nearest nodes to",
                                   cmd_find_nearest};
struct cmd_def verbose_cmd = {"verbose", "set verbosity level", cmd_verbose};
struct cmd_def save_cmd = {"save", "save the network to file", cmd_save};
struct cmd_def distance_cmd = {"distance",
                               "distance between two DHT IDs",
                               cmd_distance};
struct cmd_def bit_length_cmd = {"bit_length",
                                 "bit length of 160-bit unsigned int",
                                 cmd_bit_length};
struct cmd_def graphviz_cmd = {
    "graphviz",
    "dump a graphviz of the nodes acc/ to their k-buckets",
    cmd_graphviz};
struct cmd_def buy_storage_cmd = {"buy_storage",
                                  "buy N bytes of storage",
                                  cmd_buy_storage};
struct cmd_def put_bytes_cmd = {"put_bytes",
                                "put N bytes on storage",
                                cmd_put_bytes};
struct cmd_def get_bytes_cmd = {"get_bytes",
                                "get N bytes from storage",
                                cmd_get_bytes};

struct cmd_def* cmd_defs[] = {
    &bit_length_cmd,
    &buy_storage_cmd,
    &cheat_lookup_cmd,
    &find_nearest_cmd,
    &get_bytes_cmd,
    &graphviz_cmd,
    &help_cmd,
    &jump_cmd,
    &lookup_cmd,
    &put_bytes_cmd,
    &quit_cmd,
    &rand_node_cmd,
    &rand_key_cmd,
    &save_cmd,
    &show_cmd,
    &verbose_cmd,
    &distance_cmd,
    nullptr,
};

} // namespace kad
