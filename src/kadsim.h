#ifndef __KADSIM_H__
#define __KADSIM_H__

// Most classes do not need copy or assignment constructors and should use this
// macro to disable them.
#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                     \
    TypeName(const TypeName&);                                                 \
    void operator=(const TypeName&)

// Address of the QuadIron contract on the blockchain.
#define QUADIRON_CONTRACT_ADDR "0x5e667a8D97fBDb2D3923a55b295DcB8f5985FB79"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <random>
#include <vector>

#include <getopt.h>
#include <jsonrpccpp/client/connectors/httpclient.h>
#include <netinet/in.h>

#include "bignum.h"
#include "bit_map.h"
#include "gethclient.h"
#include "kadclient.h"
#include "shell.h"

class KadConf {
  public:
    KadConf(
        int n_bits,
        int k,
        int alpha,
        int n_nodes,
        const std::string& geth_addr,
        const std::vector<std::string>& bstraplist);
    void save(std::ostream& fout);
    int n_bits;
    u_int k;
    u_int alpha;
    u_int n_nodes;

    jsonrpc::HttpClient httpclient;
    GethClient geth;
    std::vector<std::string> bstraplist;
};

enum KadRoutableType {
    KAD_ROUTABLE_NODE,
    KAD_ROUTABLE_FILE,
};

class KadRoutable {
  public:
    KadRoutable(const CBigNum& id, enum KadRoutableType);
    ~KadRoutable();

    CBigNum get_id() const;
    bool is_remote();
    KadRoutableType get_type();
    CBigNum distance_to(const KadRoutable& other) const;
    bool operator()(const KadRoutable* first, const KadRoutable* second) const;

  protected:
    CBigNum id;
    KadRoutableType type;
    std::string addr; // Remote peer IP address, or "" if local.
    jsonrpc::HttpClient* httpclient;
    KadClient* kadc;
};

class KadNode;

class KadFile : public KadRoutable {
  public:
    KadFile(const CBigNum& id, KadNode* referencer);
    ~KadFile();
    KadNode* get_referencer();

  private:
    KadNode* referencer;

    // DISALLOW_COPY_AND_ASSIGN(KadFile);
};

class KadNode : public KadRoutable {
  public:
    KadNode(KadConf* conf, const CBigNum& id);
    KadNode(KadConf* conf, const CBigNum& id, const std::string& addr);
    ~KadNode();

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
    // DISALLOW_COPY_AND_ASSIGN(KadNode);

    KadConf* conf;

    typedef std::map<int, std::list<KadNode*>> tbucket;
    tbucket buckets;
    bool verbose;

    std::vector<KadFile*> files;
    std::string eth_passphrase;
    std::string eth_account;
};

typedef void (*tnode_callback_func)(KadNode* node, void* cb_arg);
typedef void (
    *troutable_callback_func)(const KadRoutable& routable, void* cb_arg);

class KadNetwork {
  public:
    KadNetwork(KadConf* conf);
    ~KadNetwork();

    void
    initialize_nodes(int n_initial_conn, std::vector<std::string> bstraplist);
    void initialize_files(int n_files);
    void rand_node(tnode_callback_func cb_func, void* cb_arg);
    void rand_routable(troutable_callback_func cb_func, void* cb_arg);
    KadNode* lookup_cheat(const std::string& id);
    KadNode* find_nearest_cheat(const KadRoutable& routable);
    void save(std::ostream& fout);
    void graphviz(std::ostream& fout);
    void check_files();

  private:
    // DISALLOW_COPY_AND_ASSIGN(KadNetwork);

    KadConf* conf;

    std::vector<KadNode*> nodes;
    std::map<std::string, KadNode*> nodes_map;
    std::vector<KadFile*> files;
};

extern struct cmd_def* cmd_defs[];

// Encode an integer as an uint256 according to the Ethereum Contract ABI.
// See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
std::string encode_uint256(uint64_t v);
// Address are encoded as uint160.
std::string encode_address(const std::string& addr);

void call_contract(
    GethClient& geth,
    const std::string& node_addr,
    const std::string& contract_addr,
    const std::string& payload);

// Return a reference to the global PRNG.
static inline std::mt19937& prng()
{
    static std::mt19937 PRNG;

    return PRNG;
}

#endif
