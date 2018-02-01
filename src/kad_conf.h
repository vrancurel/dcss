#ifndef __KAD_CONF_H__
#define __KAD_CONF_H__

#include <ostream>
#include <string>
#include <vector>

#include <jsonrpccpp/client/connectors/httpclient.h>

#include "gethclient.h"

class KadConf {
  public:
    KadConf(
        int n_bits,
        int k,
        int alpha,
        int n_nodes,
        const std::string& geth_addr,
        std::vector<std::string> bstraplist);
    void save(std::ostream& fout);
    int n_bits;
    u_int k;
    u_int alpha;
    u_int n_nodes;

    jsonrpc::HttpClient httpclient;
    GethClient geth;
    std::vector<std::string> bstraplist;
};

#endif
