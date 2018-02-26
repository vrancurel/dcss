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
#ifndef __KAD_NODE_H__
#define __KAD_NODE_H__

#include <cstdint>
#include <list>
#include <string>

#include <jsonrpccpp/client/connectors/httpclient.h>

#include "kad_routable.h"

class NodeClient;

namespace kad {

class CBigNum;
class Conf;
class File;

class Node : public Routable {
  public:
    Node(const Conf& configuration, const CBigNum& node_id);
    Node(
        const Conf& configuration,
        const CBigNum& node_id,
        const std::string& rpc_addr);

    ~Node() = default;
    Node(Node const&) = delete;
    Node& operator=(Node const& x) = delete;
    Node(Node&&) = delete;
    Node& operator=(Node&& x) = delete;

    uint32_t get_n_conns();
    const std::string& get_eth_account() const;
    bool add_conn(Node* node, bool contacted_us);
    std::list<Node*>
    find_nearest_nodes(const Routable& routable, uint32_t amount);
    std::list<Node*>
    find_nearest_nodes_local(const Routable& routable, uint32_t amount);
    std::list<Node*> lookup(const Routable& routable);
    void show();
    void set_verbose(bool enable);
    void save(std::ostream& fout);
    void store(File* file);
    std::vector<File*> get_files();
    void graphviz(std::ostream& fout);

    void buy_storage(const std::string& seller, uint64_t nb_bytes);
    void put_bytes(const std::string& seller, uint64_t nb_bytes);
    void get_bytes(const std::string& seller, uint64_t nb_bytes);

  private:
    const Conf* const conf;

    using tbucket = std::map<uint32_t, std::list<Node*>>;
    tbucket buckets;
    bool verbose;

    std::vector<File*> files;
    std::string eth_passphrase;
    std::string eth_account;
    jsonrpc::HttpClient* httpclient;
    NodeClient* nodec;
};

} // namespace kad

#endif
