#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <getopt.h>

#include "quadiron.h"

[[noreturn]] static void usage()
{
    std::cerr << "usage: " << PACKAGE << " (" << VERSION << ")\n";
    std::cerr << "\t-b\tn_bits\n";
    std::cerr << "\t-k\tKademlia K parameter\n";
    std::cerr << "\t-a\tKademlia alpha parameter\n";
    std::cerr << "\t-n\tnumber of nodes\n";
    std::cerr << "\t-c\tinitial number of connections per node\n";
    std::cerr << "\t-g\tgeth RPC server address\n";
    std::cerr << "\t-B\tbootstrap list (comma-separated list of IPs)\n";
    std::cerr << "\t-N\tnumber of files\n";
    std::cerr << "\t-S\trandom seed\n";
    exit(1);
}

[[noreturn]] static void parse_error()
{
    std::cerr << "parse error\n";
    exit(1);
}

int main(int argc, char** argv)
{
    int c;
    uint32_t n_bits = 64;
    uint32_t k = 20;
    uint32_t alpha = 3;
    uint32_t n_nodes = 1500;
    uint32_t n_init_conn = 100;
    uint32_t n_files = 5000;
    uint32_t rand_seed = 0;
    std::string fname;
    std::string geth_addr = "localhost:8545";
    std::vector<std::string> bstraplist;

    opterr = 0;

    while ((c = getopt(argc, argv, "b:k:a:n:c:g:B:S:f:N:")) != -1) {
        switch (c) {
        case 'b':
            n_bits = kad::stou32(optarg);
            break;
        case 'k':
            k = kad::stou32(optarg);
            break;
        case 'a':
            alpha = kad::stou32(optarg);
            break;
        case 'n':
            n_nodes = kad::stou32(optarg);
            break;
        case 'c':
            n_init_conn = kad::stou32(optarg);
            break;
        case 'g':
            geth_addr = optarg;
            break;
        case 'B': {
            std::istringstream input(optarg);
            std::string bstrap;
            while (!std::getline(input, bstrap, ',').fail()) {
                bstraplist.push_back(bstrap);
            }
            break;
        }
        case 'S':
            rand_seed = kad::stou32(optarg);
            break;
        case 'f':
            fname = optarg;
            break;
        case 'N':
            n_files = kad::stou32(optarg);
            break;
        case '?':
        default:
            usage();
        }
    }

    if (!fname.empty()) {
        std::ifstream fin(fname);
        if (!fin.is_open()) {
            std::cerr << "unable to open " << fname << "\n";
            exit(1);
        }

        char buf[256], *p;
#define GETLINE()                                                              \
    fin.getline(buf, sizeof(buf));                                             \
    if (NULL == (p = rindex(buf, ' ')))                                        \
        parse_error();                                                         \
    p++;
        GETLINE();
        n_bits = kad::stou32(p);
        GETLINE();
        k = kad::stou32(p);
        GETLINE();
        alpha = kad::stou32(p);
        GETLINE();
        n_nodes = kad::stou32(p);
    }

    kad::Conf conf(n_bits, k, alpha, n_nodes, geth_addr, bstraplist);
    // conf.save(std::cout);
    kad::Network network(conf);
    kad::Shell shell;

    kad::prng().seed(rand_seed);

    network.initialize_nodes(n_init_conn, bstraplist);
    network.initialize_files(n_files);
    network.check_files();

    shell.set_cmds(kad::cmd_defs);
    shell.set_handle(&network);
    std::ostringstream prompt;
    prompt << PACKAGE << "> ";
    shell.set_prompt(prompt.str());
    shell.loop();

    return EXIT_SUCCESS;
}
