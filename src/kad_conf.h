#ifndef __KAD_CONF_H__
#define __KAD_CONF_H__

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include <jsonrpccpp/client/connectors/httpclient.h>

#include "gethclient.h"

namespace kad {

class Conf {
  public:
    Conf(
        uint32_t nb_bits,
        uint32_t k_param,
        uint32_t alpha_param,
        uint32_t nb_nodes,
        const std::string& geth_addr,
        std::vector<std::string> bootstrap_list);

    void save(std::ostream& fout) const;

    uint32_t n_bits;
    uint32_t k;
    uint32_t alpha;
    uint32_t n_nodes;

    jsonrpc::HttpClient httpclient;
    mutable GethClient geth;
    std::vector<std::string> bstraplist;
};

} // namespace kad

#endif
