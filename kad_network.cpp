#include "kadsim.h"

KadNetwork::KadNetwork(KadConf *conf)
{
  this->conf = conf;
}

KadNetwork::~KadNetwork()
{

}

/** 
 * initialize nodes
 * 
 * @param n_init_conn 
 */
void 
KadNetwork::initialize_nodes(int n_init_conn)
{
  std::cout << "initialize nodes\n";

  BitMap bitmap = BitMap(conf->n_nodes);

  //split the total keyspace equally among nodes
  CBigNum keyspace = CBigNum(1);
  keyspace=(keyspace<<conf->n_bits);
  keyspace/=conf->n_nodes;

  //create nodes
  for (u_int i = 0;i < conf->n_nodes;i++)
    {
      KadNode *node = new KadNode(conf, bitmap.get_rand_bit()*keyspace);
      nodes.push_back(node);
      nodes_map[node->get_id().ToString(16)] = node;
    }

  //there shall be a responsable for every portion of the keyspace
  assert(!bitmap.check());

  //continue creating conns for the nodes that dont meet the initial number required
  for (u_int i = 0;i < conf->n_nodes;i++)
    {
      KadNode *node = nodes[i];

      if ((i % 1000) == 0)
	std::cerr << "node " << i << "/" << conf->n_nodes << "                   \r";

      u_int guard = 0;
      while (node->get_n_conns() < n_init_conn)
	{
	  KadNode *other;
	  
	  if (guard >= (2 * conf->n_nodes))
	    {
	      std::cout << "forgiving required conditions for " << node->get_id().ToString(16) << ", it has only " << node->get_n_conns() << " connections\n";
	      break ;
	    }

	  //pick a random node
	  int x = rand() % nodes.size();
	  other = nodes[x];
	  
	  //connect them 2-way
	  //std::cout << "connecting nodes " << node->get_id().ToString(16) << " (" << node->get_n_conns() << ") and " << other->get_id().ToString(16) << "\n";
	  node->add_conn(other, false);
	  other->add_conn(node, false);

	  guard++;
	}
    }  

  std::cout << "\n";
}

void 
KadNetwork::initialize_files(int n_files)
{
  std::cout << "initialize files\n";

  for (int i = 0;i < n_files;i++)
    {
      if ((i % 1000) == 0)
	std::cerr << "file " << i << "/" << n_files << "                   \r";

      //take a random node
      int x = rand() % nodes.size();
      KadNode *node = nodes[x];

      //gen a random identifier for the file
      CBigNum bn;
      bn.Rand(conf->n_bits);
      KadFile *file = new KadFile(bn, node);
      files.push_back(file);

      //store file at multiple location
      std::list<KadNode*> result = node->lookup(*file);
      for (std::list<KadNode*>::iterator it = result.begin();it != result.end();++it)
	(*it)->store(file);
    }
}

/**
 * check that files are accessible from random nodes
 */
void 
KadNetwork::check_files()
{
  std::cout << "checking files\n";

  int n_wrong = 0;
  for (u_int i = 0;i < files.size();i++)
    {
      if ((i % 1000) == 0)
	std::cerr << "file " << i << "/" << files.size() << "                   \r";

      KadFile *file = files[i];

      //take a random node
      int x = rand() % nodes.size();
      KadNode *node = nodes[x];

      //get results
      std::list<KadNode*> result = node->lookup(*file);

      //check that at least one result has the file
      bool found = false;
      for (std::list<KadNode*>::iterator it = result.begin();it != result.end();++it)
	{
	  std::vector<KadFile*> node_files = (*it)->get_files();
	  for (u_int j = 0;j < node_files.size();j++)
	    {
	      KadFile *node_file = node_files[j];
	      if (node_file == file)
		{
		  found = true;
		  break ;
		}
	    }
	}

      if (!found)
	{
	  std::cerr << "file " << file->get_id().ToString(16) << " who was referenced by " << file->get_referencer()->get_id().ToString(16) << " was not found\n";
	  n_wrong++;
	}
    }
  std::cerr << n_wrong << "/" << files.size() << " files wrongly stored\n";
}

void
KadNetwork::rand_node(tnode_callback_func cb_func,
		      void *cb_arg)
{
  int x = rand() % nodes.size();
  if (NULL != cb_func)
    cb_func(nodes[x], cb_arg);
}

void
KadNetwork::rand_routable(troutable_callback_func cb_func,
			  void *cb_arg)
{
  CBigNum bn;
  bn.Rand(conf->n_bits);
  KadRoutable routable(bn, KAD_ROUTABLE_FILE);
  if (NULL != cb_func)
    cb_func(routable, cb_arg);
}

/** 
 * lookup a node by its id
 * 
 * @param id 
 * 
 * @return 
 */
KadNode *
KadNetwork::lookup_cheat(std::string id)
{
  return nodes_map[id];
}

/** 
 * find node nearest to specified routable
 * 
 * @param routable 
 * 
 * @return 
 */
KadNode *
KadNetwork::find_nearest_cheat(KadRoutable routable)
{
  KadNode *nearest = NULL;

  for (u_int i = 0;i < nodes.size();i++)
    {
      if (NULL == nearest)
	nearest = nodes[i];
      else
	{
	  CBigNum d1 = nodes[i]->distance_to(routable);
	  CBigNum d2 = nearest->distance_to(routable);
	  
	  if (d1 < d2)
	    nearest = nodes[i];
	}
    }

  return nearest;
}

void
KadNetwork::save(std::ostream& fout)
{
  conf->save(fout);
  for (u_int i = 0;i < conf->n_nodes;i++)
    {
      KadNode *node = nodes[i];
      
      fout << "node " << i << " " << node->get_id().ToString(16)  << "\n";
      node->save(fout);
    }
}

void
KadNetwork::graphviz(std::ostream& fout)
{
  fout << "digraph G {\n";
  fout << "  node [shape=record];\n";
  fout << "  rankdir=TB;\n";

  for (u_int i = 0;i < conf->n_nodes;i++)
    {
      KadNode *node = nodes[i];
      
      fout << "node_" << node->get_id().ToString(16) << " [color=blue, label=\"" << node->get_id().ToString(16) << "\"];\n";
      node->graphviz(fout);
    }

  fout << "}\n";
}

void
KadNetwork::call_contract(const std::string& name, const std::string& from,
                          const std::string& payload)
{
  Json::Value params;

  params["from"] = from;
  params["to"] = ""; // TODO: lookup address from name in hashtbl
  params["data"] = payload;

  std::string tx_hash;
  try {
      tx_hash = conf->geth.eth_sendTransaction(params);
      std::cout << "tx_hash: " << tx_hash << '\n';
  } catch (jsonrpc::JsonRpcException exn) {
    fprintf(stderr, "transaction error: %s\n", exn.what());
    return;
  }

  // FIXME: busy way is ugly.
  while (true) {
      try {
          const Json::Value receipt = conf->geth.eth_getTransactionReceipt(tx_hash);
          std::cout << "result: " << receipt.toStyledString() << '\n';
          // TODO: we should probably return a bool to the caller, or raise…
          if (receipt["status"] == "0x0") {
              std::cout << "transaction failed\n";
          } else {
              std::cout << "transaction successed: " << receipt["status"] << '\n';
          }
          return;
      } catch (jsonrpc::JsonRpcException exn) {
          if (exn.GetCode() == -32000) {
              continue;  // Transaction is pending…
          } else {
              fprintf(stderr, "error: %s\n", exn.what());
              return;
          }
      }
  }
}
