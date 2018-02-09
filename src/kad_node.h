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
