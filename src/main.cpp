/*
 * Copyright 2017-2018 the QuadIron authors
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <getopt.h>
#include <nttec/nttec.h>

#include "quadiron.h"

[[noreturn]] static void show_version()
{
    std::cout << PACKAGE << " (" << VERSION << ")\n"
              << "using NTTEC " << nttec::get_version() << '\n';
    exit(0);
}

[[noreturn]] static void usage()
{
    std::cerr << "usage: " << PACKAGE << '\n';
    std::cerr << "\t-b\tn_bits\n";
    std::cerr << "\t-k\tKademlia K parameter\n";
    std::cerr << "\t-a\tKademlia alpha parameter\n";
    std::cerr << "\t-n\tnumber of nodes\n";
    std::cerr << "\t-c\tinitial number of connections per node\n";
    std::cerr << "\t-g\tgeth RPC server address\n";
    std::cerr << "\t-B\tbootstrap list (comma-separated list of IPs)\n";
    std::cerr << "\t-N\tnumber of files\n";
    std::cerr << "\t-S\trandom seed\n";
    std::cerr << "\t-V\tshow version\n";
    exit(1);
}

[[noreturn]] static void parse_error()
{
    std::cerr << "parse error\n";
    exit(1);
}

/** Setup the logging system and initialize the loggers.
 *
 * @return 0 on success, -1 on error.
 */
static int setup_logging()
{
#define TIME_FMT "%datetime{%Y-%M-%dT%H:%m:%s}"
    const char* std_fmt = TIME_FMT " [%level] %logger: %msg";
    const char* dbg_fmt = TIME_FMT " [%level] %logger (%loc): %msg";
    const char* vvv_fmt = TIME_FMT " [%level-%vlevel] %logger (%loc): %msg";
#undef TIME_FMT

    // Apply default configuration to all the loggers.
    el::Configurations log_cfg;
    log_cfg.setToDefault();
    log_cfg.set(el::Level::Global, el::ConfigurationType::Format, std_fmt);
    log_cfg.set(el::Level::Global, el::ConfigurationType::ToFile, "false");
    log_cfg.set(el::Level::Debug, el::ConfigurationType::Format, dbg_fmt);
    log_cfg.set(el::Level::Verbose, el::ConfigurationType::Format, vvv_fmt);
    log_cfg.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
    log_cfg.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");

    const char* logger_ids[] = {SIM_LOG_ID, ETH_LOG_ID};
    for (const auto& log_id : logger_ids) {
        const el::Logger* logger = el::Loggers::getLogger(log_id);

        if (logger == nullptr) {
            return -1;
        }
        el::Loggers::reconfigureLogger(log_id, log_cfg);
    }

    return 0;
}

// NOLINTNEXTLINE(cert-err58-cpp)
INITIALIZE_EASYLOGGINGPP

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

    START_EASYLOGGINGPP(argc, argv);

    opterr = 0;

    while ((c = getopt(argc, argv, "b:k:a:n:c:g:B:S:f:N:V")) != -1) {
        switch (c) {
        case 'b':
            n_bits = kad::stou32(optarg);
            if (n_bits > 160) {
                std::cerr << "cannot support more than 160 bits\n";
                exit(1);
            }
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
        case 'V':
            show_version();
        case '?':
            break; // Could be options for easyloggingpp.
        default:
            usage();
        }
    }

    if (setup_logging() < 0) {
        std::cerr << "cannot setup the logging system\n";
        return EXIT_FAILURE;
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
    shell.set_prompt(std::string(PACKAGE) + "> ");
    shell.loop();

    return EXIT_SUCCESS;
}
