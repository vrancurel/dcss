#ifndef __KADSIM_H__
#define __KADSIM_H__

// Most classes do not need copy or assignment constructors and should use this
// macro to disable them.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);		   \
  void operator=(const TypeName&)

// Address of the QuadIron contract on the blockchain.
#define QUADIRON_CONTRACT_ADDR  "0x5e667a8D97fBDb2D3923a55b295DcB8f5985FB79"

#include <cstdint>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <getopt.h>

#include <jsonrpccpp/client/connectors/httpclient.h>

#include "bignum.h"
#include "bit_map.h"
#include "gethclient.h"
#include "shell.h"

class KadConf
{
 public:
  KadConf(int n_bits, int k, int alpha, int n_nodes,
          const std::string &geth_addr);
  void save(std::ostream& fout);
  int n_bits;
  u_int k;
  u_int alpha;
  u_int n_nodes;

  jsonrpc::HttpClient httpclient;
  GethClient geth;
};

enum KadRoutableType
  {
    KAD_ROUTABLE_NODE,
    KAD_ROUTABLE_FILE,
  };

class KadRoutable
{
 public:
  KadRoutable(CBigNum id, enum KadRoutableType);
  ~KadRoutable();

  CBigNum get_id() const;
  std::string get_name();
  KadRoutableType get_type();
  CBigNum distance_to(KadRoutable other) const;
  bool operator()(const KadRoutable *first, const KadRoutable *second) const;

 protected:

  CBigNum id;
  KadRoutableType type;
  std::string name;
  
};

class KadNode;

class KadFile : public KadRoutable
{
 public:
  KadFile(CBigNum id, KadNode *referencer);
  ~KadFile();
  KadNode *get_referencer();

 private:

  KadNode *referencer;

  //DISALLOW_COPY_AND_ASSIGN(KadFile);
};

class KadNode : public KadRoutable
{
 public:
  KadNode(KadConf *conf, CBigNum id);
  ~KadNode();

  int get_n_conns();
  const std::string& get_eth_account() const;
  bool add_conn(KadNode *node, bool contacted_us);
  std::list<KadNode*> find_nearest_nodes(KadRoutable routable, int amount);
  std::list<KadNode*> lookup(KadRoutable routable);
  void show();
  void set_verbose(int level);
  void save(std::ostream& fout);
  void store(KadFile *file);
  std::vector<KadFile*> get_files();
  void graphviz(std::ostream& fout);

 private:
  //DISALLOW_COPY_AND_ASSIGN(KadNode);

  KadConf *conf;

  typedef std::map<int,std::list<KadNode*> > tbucket;
  tbucket buckets;
  int verbose;

  std::vector<KadFile*> files;
  std::string eth_passphrase;
  std::string eth_account;
};

typedef void (*tnode_callback_func)(KadNode *node, void *cb_arg);
typedef void (*troutable_callback_func)(KadRoutable routable, void *cb_arg);

class KadNetwork
{
 public:
  KadNetwork(KadConf *conf);
  ~KadNetwork();

  void initialize_nodes(int n_initial_conn);
  void initialize_files(int n_files);
  void rand_node(tnode_callback_func cb_func, void *cb_arg);
  void rand_routable(troutable_callback_func cb_func, void *cb_arg);
  KadNode *lookup_cheat(std::string id);
  KadNode *find_nearest_cheat(KadRoutable routable);
  void save(std::ostream& fout);
  void graphviz(std::ostream& fout);
  void check_files();

 private:
  //DISALLOW_COPY_AND_ASSIGN(KadNetwork);

  KadConf *conf;

  std::vector<KadNode*> nodes;
  std::map<std::string,KadNode*> nodes_map;
  std::vector<KadFile*> files;

};

extern struct cmd_def *cmd_defs[];

// Encode an integer as an uint256 according to the Ethereum Contract ABI.
// See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
std::string encode_uint256(uint64_t v);
// Address are encoded as uint160
std::string encode_address(const std::string &addr);

void call_contract(GethClient &geth,
                   const std::string &node_addr,
                   const std::string &contract_addr,
                   const std::string &payload);

#endif
