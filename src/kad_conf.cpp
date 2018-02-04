#include "kad_conf.h"

KadConf::KadConf(
    uint32_t nb_bits,
    uint32_t k_param,
    uint32_t alpha_param,
    uint32_t nb_nodes,
    const std::string& geth_addr,
    std::vector<std::string> bootstrap_list)
    : httpclient(geth_addr), geth(httpclient),
      bstraplist(std::move(bootstrap_list))
{
    this->n_bits = nb_bits;
    this->k = k_param;
    this->alpha = alpha_param;
    this->n_nodes = nb_nodes;
}

void KadConf::save(std::ostream& fout)
{
    fout << "n_bits " << n_bits << '\n'
         << "k " << k << '\n'
         << "alpha " << alpha << '\n'
         << "n_nodes " << n_nodes << '\n';
}
