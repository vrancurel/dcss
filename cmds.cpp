
#include "kadsim.h"

int
cmd_quit(Shell *shell,
	 int argc,
	 char **argv)
{
  return SHELL_RETURN;
}

int
cmd_help(Shell *shell,
	 int argc,
	 char **argv)
{
  if (argc == 1)
    {
      struct cmd_def *cmdp;
      int i, j;

      j = 0;
      for (i = 0;cmd_defs[i];i++)
	{
          cmdp = cmd_defs[i];
	  printf("%16s", cmdp->name);
	  j++;
	  if (j == 4)
	    {
	      printf("\n");
	      j = 0;
	    }
	}
      printf("\n");
    }
  else if (argc == 2)
    {
      struct cmd_def *cmdp;
      int i;

      for (i = 0;cmd_defs[i];i++)
	{
          cmdp = cmd_defs[i];
	  if (!strcmp(argv[1], cmdp->name))
	    {
	      printf("%s\n", cmdp->purpose);
	      break ;
	    }
	}
    }

  return SHELL_CONT;
}

void
cb_display_node(KadNode *node,
	  void *cb_arg)
{
  std::cout << node->get_id().ToString(16) << "\n";
}

void
cb_display_routable(KadRoutable routable,
		    void *cb_arg)
{
  std::cout << routable.get_id().ToString(16) << "\n";
}

int
cmd_rand_node(Shell *shell,
	      int argc,
	      char **argv)
{
  KadNetwork *network = (KadNetwork *) shell->get_handle();

  network->rand_node(cb_display_node, NULL);

  return SHELL_CONT;
}

int
cmd_rand_key(Shell *shell,
	      int argc,
	      char **argv)
{
  KadNetwork *network = (KadNetwork *) shell->get_handle();

  network->rand_routable(cb_display_routable, NULL);

  return SHELL_CONT;
}

int
cmd_jump(Shell *shell,
	 int argc,
	 char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: jump key\n");
      return SHELL_CONT;
    }

  KadNetwork *network = (KadNetwork *) shell->get_handle();

  KadNode *node = network->lookup_cheat(argv[1]);
  if (NULL == node)
    {
      fprintf(stderr, "not found\n");
      return SHELL_CONT;
    }

  shell->set_handle2(node);
  
  return SHELL_CONT;
}

int
cmd_lookup(Shell *shell,
	   int argc,
	   char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: lookup key\n");
      return SHELL_CONT;
    }

  KadNode *node = (KadNode *) shell->get_handle2();

  if (NULL == node)
    {
      fprintf(stderr, "shall jump to a node first\n");
      return SHELL_CONT;
    }

  CBigNum bn;
  bn.SetHex(argv[1]);
  KadRoutable routable(bn, KAD_ROUTABLE_FILE);

  //std::cout << bn.ToString(16) << "\n";

  std::list<KadNode*> result = node->lookup(routable);

  std::list<KadNode*>::iterator it;
  for (it = result.begin();it != result.end();++it)
    std::cout << "id " << (*it)->get_id().ToString(16) << " dist " << (*it)->distance_to(routable).ToString(16) << "\n";

  return SHELL_CONT;
}

int
cmd_find_nearest(Shell *shell,
		 int argc,
		 char **argv)
{
  if (argc != 3)
    {
      fprintf(stderr, "usage: find_nearest key amount\n");
      return SHELL_CONT;
    }

  KadNode *node = (KadNode *) shell->get_handle2();

  if (NULL == node)
    {
      fprintf(stderr, "shall jump to a node first\n");
      return SHELL_CONT;
    }

  CBigNum bn;
  bn.SetHex(argv[1]);
  KadRoutable routable(bn, KAD_ROUTABLE_FILE);

  //std::cout << bn.ToString(16) << "\n";

  std::list<KadNode*> result = node->find_nearest_nodes(routable, atoi(argv[2]));

  std::list<KadNode*>::iterator it;
  for (it = result.begin();it != result.end();++it)
    std::cout << "id " << (*it)->get_id().ToString(16) << " dist " << (*it)->distance_to(routable).ToString(16) << "\n";

  return SHELL_CONT;
}

int
cmd_show(Shell *shell,
	 int argc,
	 char **argv)
{
  if (argc != 1)
    {
      fprintf(stderr, "usage: show\n");
      return SHELL_CONT;
    }

  KadNode *node = (KadNode *) shell->get_handle2();

  if (NULL == node)
    {
      fprintf(stderr, "shall jump to a node first\n");
      return SHELL_CONT;
    }

  node->show();

  return SHELL_CONT;
}

int
cmd_verbose(Shell *shell,
	    int argc,
	    char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: verbose 1|0\n");
      return SHELL_CONT;
    }

  KadNode *node = (KadNode *) shell->get_handle2();

  if (NULL == node)
    {
      fprintf(stderr, "shall jump to a node first\n");
      return SHELL_CONT;
    }

  node->set_verbose(atoi(argv[1]));

  return SHELL_CONT;
}

int
cmd_cheat_lookup(Shell *shell,
		 int argc,
		 char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: jump key\n");
      return SHELL_CONT;
    }

  KadNetwork *network = (KadNetwork *) shell->get_handle();

  CBigNum bn;
  bn.SetHex(argv[1]);
  KadRoutable routable(bn, KAD_ROUTABLE_FILE);

  KadNode *node = network->find_nearest_cheat(routable);
  if (NULL == node)
    {
      fprintf(stderr, "not found\n");
      return SHELL_CONT;
    }
  
  cb_display_node(node, NULL);
  
  return SHELL_CONT;
}

int
cmd_save(Shell *shell,
	 int argc,
	 char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: save file\n");
      return SHELL_CONT;
    }

  KadNetwork *network = (KadNetwork *) shell->get_handle();

  std::ofstream fout(argv[1]);
  network->save(fout);
  
  return SHELL_CONT;
}

int
cmd_xor(Shell *shell,
	int argc,
	char **argv)
{
  if (argc != 3)
    {
      fprintf(stderr, "usage: xor bn1 bn2\n");
      return SHELL_CONT;
    }

  CBigNum bn1;
  bn1.SetHex(argv[1]);
  KadRoutable routable1(bn1, KAD_ROUTABLE_FILE);

  CBigNum bn2;
  bn2.SetHex(argv[2]);
  KadRoutable routable2(bn2, KAD_ROUTABLE_FILE);

  std::cout << routable1.distance_to(routable2).ToString(16) << "\n";

  return SHELL_CONT;
}

int
cmd_bit_length(Shell *shell,
	       int argc,
	       char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: bit_length bn\n");
      return SHELL_CONT;
    }

  CBigNum bn;
  bn.SetHex(argv[1]);

  std::cout << bn.bit_length() << "\n";

  return SHELL_CONT;
}

int
cmd_graphviz(Shell *shell,
	     int argc,
	     char **argv)
{
  if (argc != 2)
    {
      fprintf(stderr, "usage: graphviz file\n");
      return SHELL_CONT;
    }

  KadNetwork *network = (KadNetwork *) shell->get_handle();

  std::ofstream fout(argv[1]);
  network->graphviz(fout);

  return SHELL_CONT;
}

int
cmd_call_contract(Shell *shell,
	     int argc,
	     char **argv)
{
  if (argc != 4)
    {
      fprintf(stderr, "usage: call_contract NAME FROM PAYLOAD\n");
      return SHELL_CONT;
    }

  KadNetwork *network = (KadNetwork *) shell->get_handle();

  network->call_contract(argv[1], argv[2], argv[3]);

  return SHELL_CONT;
}

struct cmd_def quit_cmd = {"quit", "quit program", cmd_quit};
struct cmd_def help_cmd = {"help", "help", cmd_help};
struct cmd_def jump_cmd = {"jump", "jump to a node", cmd_jump};
struct cmd_def lookup_cmd = {"lookup", "lookup a node", cmd_lookup};
struct cmd_def cheat_lookup_cmd = {"cheat_lookup", "lookup the closest node by cheating", cmd_cheat_lookup};
struct cmd_def rand_node_cmd = {"rand_node", "print a random node id", cmd_rand_node};
struct cmd_def rand_key_cmd = {"rand_key", "print a random key", cmd_rand_key};
struct cmd_def show_cmd = {"show", "show k-buckets", cmd_show};
struct cmd_def find_nearest_cmd = {"find_nearest", "find nearest nodes to", cmd_find_nearest};
struct cmd_def verbose_cmd = {"verbose", "set verbosity level", cmd_verbose};
struct cmd_def save_cmd = {"save", "save the network to file", cmd_save};
struct cmd_def xor_cmd = {"xor", "xor between 2 bignums", cmd_xor};
struct cmd_def bit_length_cmd = {"bit_length", "bit length of bignum", cmd_bit_length};
struct cmd_def graphviz_cmd = {"graphviz", "dump a graphviz of the nodes acc/ to their k-buckets", cmd_graphviz};
struct cmd_def call_contract_cmd = {"call_contract", "call a contract", cmd_call_contract};

struct cmd_def	*cmd_defs[] =
  {
    &bit_length_cmd,
    &cheat_lookup_cmd,
    &find_nearest_cmd,
    &graphviz_cmd,
    &help_cmd,   
    &jump_cmd,
    &lookup_cmd,
    &quit_cmd,
    &rand_node_cmd,
    &rand_key_cmd,
    &save_cmd,
    &show_cmd,
    &call_contract_cmd,
    &verbose_cmd,
    &xor_cmd,
    NULL,
  };

