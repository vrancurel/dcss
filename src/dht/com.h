/*
 * Copyright 2017-2018 the DCSS authors
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
#ifndef __DCSS_DHT_COM_H__
#define __DCSS_DHT_COM_H__

#include "address.h"

namespace dcss {
namespace dht {

/** Abstract class for inter-node communication. */
class NodeComBase {
  public:
    virtual ~NodeComBase() = default;

    /** Probe a node to check if it is online.
     *
     * @param addr address of the node to probe.
     * @return true if the node is online, false if it is offline.
     */
    virtual bool ping(const NodeAddress& addr) = 0;

    /** Return the `k` nodes, amongst the known nodes, closest to node
     * `target_id`.
     *
     * @param addr      address of the node to query
     * @param target_id the targeted node
     * @param nb_nodes  the number of node to return
     * @return the list of the `nb_nodes` nodes that are the closest to
     * `target_id`.
     *
     * @note less than `nb_nodes` node can be returned (if the node knowns less
     * than `nb_nodes` node).
     */
    virtual std::vector<NodeAddress> find_node(
        const NodeAddress& addr,
        const UInt160& target_id,
        uint32_t nb_nodes) = 0;

    NodeComBase() = default;
    NodeComBase(NodeComBase const&) = default;
    NodeComBase& operator=(NodeComBase const& x) = default;
    NodeComBase(NodeComBase&&) = default;
    NodeComBase& operator=(NodeComBase&& x) = default;
};

} // namespace dht
} // namespace dcss

#endif
