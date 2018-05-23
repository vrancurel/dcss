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
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include "kad_conf.h"
#include "nodeclient.h"
#include "uint160.h"

namespace kad {

// Address of the QuadIron contract on the blockchain.
#define QUADIRON_CONTRACT_ADDR "0x5e667a8D97fBDb2D3923a55b295DcB8f5985FB79"

static inline void call_contract(
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
    ETH_LOG(TRACE) << "tx_hash=" << tx_hash;

    // FIXME: busy wait is ugly.
    while (true) {
        try {
            const Json::Value receipt = geth.eth_getTransactionReceipt(tx_hash);
            ETH_LOG(TRACE) << "tx_receipt=" << receipt.toStyledString();

            // TODO: we should probably return a bool to the caller, or raise…
            if (receipt["status"] == "0x0") {
                ETH_LOG(WARNING) << "transaction " << tx_hash << " failed";
            } else {
              ETH_LOG(TRACE) << "transaction " << tx_hash
                             << " successed: status=" << receipt["status"];
            }
            return;
        } catch (jsonrpc::JsonRpcException& exn) {
            if (exn.GetCode() == -32000) {
                continue; // Transaction is pending…
            }
            ETH_LOG(ERROR) << exn.what();
            throw;
        }
    }
}

// Encode an integer as an uint256 according to the Ethereum Contract ABI.
// See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI:
//
// > uint<M>: enc(X) is the big-endian encoding of X, padded on the
// > higher-order (left) side with zero-bytes such that the length is a
// > multiple of 32 bytes.
static inline std::string encode_uint256(uint64_t v)
{
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(64) << std::hex << v;
    return oss.str();
}

// Address are encoded as uint160.
static inline std::string encode_address(const std::string& addr)
{
    std::ostringstream oss;
    // Skip the leading 0x, pad for 160 bytes.
    oss << std::setfill('0') << std::setw(40) << addr.substr(2);
    return oss.str();
}

// NOLINTNEXTLINE(hicpp-member-init)  (because of FIXME)
template <typename NodeCom>
Node<NodeCom>::Node(const Conf& configuration, const dht::NodeAddress& addr, const NodeCom& com_iface)
  : dht::Node<NodeCom>(addr, configuration, com_iface), conf(&configuration)
{
    verbose = false;
    // FIXME: to be reworked.
#if 0
    if (addr.ip() != "127.0.0.1") {
        // TODO: use port
        this->httpclient = new jsonrpc::HttpClient(addr.ip());
        this->nodec = new NodeClient(*this->httpclient);
    }
#endif

    // The passphrase of the account is the hex of the node ID.
    this->eth_passphrase = this->id().to_string();
    try {
        this->eth_account = conf->geth.personal_newAccount(eth_passphrase);
        conf->geth.personal_unlockAccount(eth_account, eth_passphrase, 0);
    } catch (jsonrpc::JsonRpcException&) {
        this->eth_account = "";
    }
}

template <typename NodeCom>
const std::string& Node<NodeCom>::get_eth_account() const
{
    return eth_account;
}


// FIXME: To be reworked.
#if 0
template <typename NodeCom>
std::vector<Node*>
std::list<Node*>
Node::find_node(const UInt160& target_id, uint32_t nb_nodes)
{
    // FIXME: this test should be useless.
    if (!this->m_addr.ip() != "127.0.0.1") {
        Json::Value params;
        params["to"] = target_id.to_string();
        params["amount"] = nb_nodes;
        Json::Value val = this->nodec->find_nearest_nodes(params);

        return std::vector<Node*>();
    }
    return dht::Node::find_node(target_id, nb_nodes);
}
#endif

template <typename NodeCom>
void Node<NodeCom>::on_store(const dht::Entry& entry)
{
    m_file_keys.push_back(entry.key());
}

template <typename NodeCom>
const std::vector<UInt160>& Node<NodeCom>::files() const
{
    return m_file_keys;
}

template <typename NodeCom>
void Node<NodeCom>::show()
{
    std::cout << "id " << this->id() << "\n";
    std::cout << "eth_account " << get_eth_account() << "\n";
    std::cout << "n_conns " << this->connection_count() << "\n";
    save(std::cout);
}

template <typename NodeCom>
void Node<NodeCom>::set_verbose(bool enable)
{
    this->verbose = enable;
}

template <typename NodeCom>
void Node<NodeCom>::save(std::ostream& fout)
{
    for (uint32_t i = 1; i < (conf->n_bits + 1); i++) {
        const auto k_bucket = this->buckets().at(i);

        if (!k_bucket.empty()) {
            fout << "bucket " << i << "\n";
            for (auto& node : k_bucket) {
                fout << node.id() << "\n";
            }
        }
    }

    fout << "files\n";
    for (std::vector<UInt160>::size_type i = 1; i < m_file_keys.size(); i++) {
        fout << m_file_keys[i] << "\n";
    }
}

template <typename NodeCom>
void Node<NodeCom>::graphviz(std::ostream& fout)
{
    for (uint32_t i = 1; i < (conf->n_bits + 1); i++) {
        const auto k_bucket = this->buckets().at(i);

        if (!k_bucket.empty()) {
            for (auto& node : k_bucket) {
                fout << "node_" << this->id() << " -> node_"
                     << node.id() << ";\n";
            }
        }
    }
}

// TODO: factorize all of this
template <typename NodeCom>
void Node<NodeCom>::buy_storage(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyStorage(address,address,uint256)").substr(0, 8)
    const std::string selector = "0x366e4d";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        ETH_LOG(INFO) << eth_account
                      << ": buy " << nb_bytes << " bytes from " << seller;
        call_contract(conf->geth, eth_account, QUADIRON_CONTRACT_ADDR, payload);
    } catch (jsonrpc::JsonRpcException& exn) {
        ETH_LOG(ERROR) << "cannot buy " << nb_bytes << "bytes from " << seller
                       << ": " << exn.what() << '\n';
    }
}

template <typename NodeCom>
void Node<NodeCom>::put_bytes(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyerPutBytes(address,address,uint256)").substr(0, 8)
    const std::string selector = "0xed69a0";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        ETH_LOG(INFO) << eth_account
                      << ": put " << nb_bytes << " bytes from " << seller;
        call_contract(conf->geth, eth_account, QUADIRON_CONTRACT_ADDR, payload);
    } catch (jsonrpc::JsonRpcException& exn) {
        ETH_LOG(ERROR) << "cannot put " << nb_bytes << "bytes from " << seller
                       << ": " << exn.what() << '\n';
    }
}

template <typename NodeCom>
void Node<NodeCom>::get_bytes(const std::string& seller, uint64_t nb_bytes)
{
    // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    // web3.sha3("buyerGetBytes(address,address,uint256)").substr(0, 8)
    const std::string selector = "0xdad071";
    const std::string payload = selector + encode_address(eth_account)
                                + encode_address(seller)
                                + encode_uint256(nb_bytes);

    try {
        ETH_LOG(INFO) << eth_account
                      << ": get " << nb_bytes << " bytes from " << seller;
    } catch (jsonrpc::JsonRpcException& exn) {
        ETH_LOG(ERROR) << "cannot get " << nb_bytes << "bytes from " << seller
                       << ": " << exn.what() << '\n';
    }
}

} // namespace kad
