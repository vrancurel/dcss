#include <iomanip>
#include <sstream>

#include "kadsim.h"

KadConf::KadConf(
    int n_bits,
    int k,
    int alpha,
    int n_nodes,
    const std::string& geth_addr,
    const std::vector<std::string> bstraplist)
    : httpclient(geth_addr), geth(httpclient), bstraplist(bstraplist)
{
    this->n_bits = n_bits;
    this->k = k;
    this->alpha = alpha;
    this->n_nodes = n_nodes;
}

void KadConf::save(std::ostream& fout)
{
    fout << "n_bits " << n_bits << "\n";
    fout << "k " << k << "\n";
    fout << "alpha " << alpha << "\n";
    fout << "n_nodes " << n_nodes << "\n";
}

void call_contract(
    GethClient& geth,
    const std::string& node_addr,
    const std::string& contract_addr,
    const std::string& payload)
{
    Json::Value params;

    params["from"] = node_addr;
    params["to"] = contract_addr;
    params["data"] = payload;

    const std::string tx_hash = geth.eth_sendTransaction(params);
    std::cout << "tx_hash: " << tx_hash << '\n';

    // FIXME: busy way is ugly.
    while (true) {
        try {
            const Json::Value receipt = geth.eth_getTransactionReceipt(tx_hash);
            std::cout << "result: " << receipt.toStyledString() << '\n';
            // TODO: we should probably return a bool to the caller, or raise…
            if (receipt["status"] == "0x0") {
                std::cout << "transaction failed\n";
            } else {
                std::cout << "transaction successed: " << receipt["status"]
                          << '\n';
            }
            return;
        } catch (jsonrpc::JsonRpcException& exn) {
            if (exn.GetCode() == -32000) {
                continue; // Transaction is pending…
            } else {
                fprintf(stderr, "error: %s\n", exn.what());
                throw;
            }
        }
    }
}

// From https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI:
//
// > uint<M>: enc(X) is the big-endian encoding of X, padded on the
// > higher-order (left) side with zero-bytes such that the length is a
// > multiple of 32 bytes.
std::string encode_uint256(uint64_t v)
{
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(64) << std::hex << v;
    return oss.str();
}

std::string encode_address(const std::string& addr)
{
    std::ostringstream oss;
    // Skip the leading 0x, pad for 160 bytes.
    oss << std::setfill('0') << std::setw(40) << addr.substr(2);
    return oss.str();
}

void usage()
{
    std::cerr << "usage: kadsim\n";
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

void parse_error()
{
    std::cerr << "parse error\n";
    exit(1);
}

int main(int argc, char** argv)
{
    int c;
    int n_bits = 64;
    int k = 20;
    int alpha = 3;
    int n_nodes = 1500;
    int n_init_conn = 100;
    int n_files = 5000;
    int rand_seed = 0;
    std::string fname;
    std::string geth_addr = "localhost:8545";
    std::vector<std::string> bstraplist;

    opterr = 0;

    while ((c = getopt(argc, argv, "b:k:a:n:c:g:B:S:f:N:")) != -1) {
        switch (c) {
        case 'b':
            n_bits = std::stoi(optarg);
            break;
        case 'k':
            k = std::stoi(optarg);
            break;
        case 'a':
            alpha = std::stoi(optarg);
            break;
        case 'n':
            n_nodes = std::stoi(optarg);
            break;
        case 'c':
            n_init_conn = std::stoi(optarg);
            break;
        case 'g':
            geth_addr = optarg;
            break;
        case 'B': {
            std::istringstream input(optarg);
            std::string bstrap;
            while (std::getline(input, bstrap, ',')) {
                bstraplist.push_back(bstrap);
            }
            break;
        }
        case 'S':
            rand_seed = std::stoi(optarg);
            break;
        case 'f':
            fname = optarg;
            break;
        case 'N':
            n_files = std::stoi(optarg);
            break;
        case '?':
        default:
            usage();
        }
    }

    if (fname.empty()) {
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
        n_bits = std::stoi(p);
        GETLINE();
        k = std::stoi(p);
        GETLINE();
        alpha = std::stoi(p);
        GETLINE();
        n_nodes = std::stoi(p);
    }

    KadConf conf(n_bits, k, alpha, n_nodes, geth_addr, bstraplist);
    // conf.save(std::cout);
    KadNetwork network(&conf);
    Shell shell;

    prng().seed(rand_seed);

    network.initialize_nodes(n_init_conn, bstraplist);
    network.initialize_files(n_files);
    network.check_files();

    shell.set_cmds(cmd_defs);
    shell.set_handle(&network);
    shell.set_prompt("kadsim> ");
    shell.loop();

    return EXIT_SUCCESS;
}
