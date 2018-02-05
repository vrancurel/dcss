#ifndef __KAD_NETWORK_H__
#define __KAD_NETWORK_H__

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace kad {

class Conf;
class File;
class Node;
class Routable;

using tnode_callback_func = void (*)(Node*, void*);
using troutable_callback_func = void (*)(const Routable&, void*);

class Network {
  public:
    explicit Network(Conf* configuration);

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
    void rand_routable(troutable_callback_func cb_func, void* cb_arg);
    Node* lookup_cheat(const std::string& id);
    Node* find_nearest_cheat(const Routable& routable);
    void save(std::ostream& fout);
    void graphviz(std::ostream& fout);
    void check_files();

  private:
    Conf* conf;

    std::vector<Node*> nodes;
    std::map<std::string, Node*> nodes_map;
    std::vector<File*> files;
};

} // namespace kad

#endif
