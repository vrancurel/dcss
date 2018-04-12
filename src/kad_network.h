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
#ifndef __KAD_NETWORK_H__
#define __KAD_NETWORK_H__

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "kad_node.h"
#include "kad_node_com.h"
#include "uint160.h"

namespace kad {

class Conf;

using tnode_callback_func = void (*)(const Node<NodeLocalCom>&, void*);
using tkey_callback_func = void (*)(const UInt160&, void*);

class Network {
  public:
    explicit Network(const Conf& configuration);

    ~Network() = default;
    Network(Network const&) = delete;
    Network& operator=(Network const& x) = delete;
    Network(Network&&) = delete;
    Network& operator=(Network&& x) = delete;

    void initialize_nodes(
        uint32_t n_initial_conn,
        std::vector<std::string> bstraplist);
    void initialize_files(uint32_t n_files);
    void rand_node(tnode_callback_func cb_func, void* cb_arg);
    void rand_key(tkey_callback_func cb_func, void* cb_arg);
    Node<NodeLocalCom>* lookup_cheat(const std::string& id) const;
    Node<NodeLocalCom>* find_nearest_cheat(const UInt160& target_id);
    void save(std::ostream& fout);
    void graphviz(std::ostream& fout);
    void check_files();

  private:
    const Conf* const conf;

    std::vector<Node<NodeLocalCom>*> nodes;
    std::map<std::string, Node<NodeLocalCom>*> nodes_map;
    std::vector<UInt160> files;
};

} // namespace kad

#endif
