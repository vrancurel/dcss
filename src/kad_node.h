#ifndef __KAD_NODE_H__
#define __KAD_NODE_H__

#include <list>
#include <string>

#include <jsonrpccpp/client/connectors/httpclient.h>

#include "kad_routable.h"

class CBigNum;
class KadConf;
class KadClient;
class KadFile;

class KadNode : public KadRoutable {
  public:
    KadNode(KadConf* conf, const CBigNum& id);
    KadNode(KadConf* conf, const CBigNum& id, const std::string& addr);

    ~KadNode() = default;
    KadNode(KadNode const&) = delete;
    KadNode& operator=(KadNode const& x) = delete;
    KadNode(KadNode&&) = delete;
    KadNode& operator=(KadNode&& x) = delete;

    int get_n_conns();
    const std::string& get_eth_account() const;
    bool add_conn(KadNode* node, bool contacted_us);
    std::list<KadNode*>
    find_nearest_nodes(const KadRoutable& routable, int amount);
    std::list<KadNode*>
    find_nearest_nodes_local(const KadRoutable& routable, int amount);
    std::list<KadNode*> lookup(const KadRoutable& routable);
    void show();
    void set_verbose(bool enable);
    void save(std::ostream& fout);
    void store(KadFile* file);
    std::vector<KadFile*> get_files();
    void graphviz(std::ostream& fout);

    void buy_storage(const std::string& seller, uint64_t nb_bytes);
    void put_bytes(const std::string& seller, uint64_t nb_bytes);
    void get_bytes(const std::string& seller, uint64_t nb_bytes);

  private:
    KadConf* conf;

    using tbucket = std::map<int, std::list<KadNode*>>;
    tbucket buckets;
    bool verbose;

    std::vector<KadFile*> files;
    std::string eth_passphrase;
    std::string eth_account;
    jsonrpc::HttpClient* httpclient;
    KadClient* kadc;
};

#endif
