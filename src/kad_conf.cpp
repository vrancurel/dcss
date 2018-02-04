#include "kad_conf.h"

KadConf::KadConf(
    uint32_t n_bits,
    uint32_t k,
    uint32_t alpha,
    uint32_t n_nodes,
    const std::string& geth_addr,
    std::vector<std::string> bstraplist)
    : httpclient(geth_addr), geth(httpclient), bstraplist(std::move(bstraplist))
{
    this->n_bits = n_bits;
    this->k = k;
    this->alpha = alpha;
    this->n_nodes = n_nodes;
}

void KadConf::save(std::ostream& fout)
{
    fout << "n_bits " << n_bits << '\n'
         << "k " << k << '\n'
         << "alpha " << alpha << '\n'
         << "n_nodes " << n_nodes << '\n';
}
