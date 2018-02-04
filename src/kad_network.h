#ifndef __KAD_NETWORK_H__
#define __KAD_NETWORK_H__

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <vector>

class KadConf;
class KadFile;
class KadNode;
class KadRoutable;

using tnode_callback_func = void (*)(KadNode*, void*);
using troutable_callback_func = void (*)(const KadRoutable&, void*);

class KadNetwork {
  public:
    explicit KadNetwork(KadConf* conf);

    ~KadNetwork() = default;
    KadNetwork(KadNetwork const&) = delete;
    KadNetwork& operator=(KadNetwork const& x) = delete;
    KadNetwork(KadNetwork&&) = delete;
    KadNetwork& operator=(KadNetwork&& x) = delete;

    void initialize_nodes(
        uint32_t n_initial_conn,
        std::vector<std::string> bstraplist);
    void initialize_files(uint32_t n_files);
    void rand_node(tnode_callback_func cb_func, void* cb_arg);
    void rand_routable(troutable_callback_func cb_func, void* cb_arg);
    KadNode* lookup_cheat(const std::string& id);
    KadNode* find_nearest_cheat(const KadRoutable& routable);
    void save(std::ostream& fout);
    void graphviz(std::ostream& fout);
    void check_files();

  private:
    KadConf* conf;

    std::vector<KadNode*> nodes;
    std::map<std::string, KadNode*> nodes_map;
    std::vector<KadFile*> files;
};

#endif
