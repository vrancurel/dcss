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
#ifndef __KAD_DHT_NODE_H__
#define __KAD_DHT_NODE_H__

#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "address.h"

namespace kad {

class Conf;

namespace dht {

/** A node of the DHT. */
template <typename NodeCom>
class Node {
  public:
    /** Create a new node.
     *
     * @param addr          Node address
     * @param configuration Kademlia configuration
     * @param com_iface     Inter-node communication interface
     */
    Node(NodeAddress addr, const Conf& configuration, const NodeCom& com_iface);

    virtual ~Node() = default;

    /* Probe the node identified by `node_id` to see if it is still online.
     *
     * @param addr address of the node to probe.
     */
    void ping(const Node& node);

    /** Return the `k` nodes, amongst the known nodes, closest to node
     * `target_id`.
     *
     * @param target_id the targeted node.
     * @param nb_nodes  the number of node to return.
     * @return the list of the `nb_nodes` nodes that are the closest to
     * `target_id`.
     *
     * @note less than `nb_nodes` node can be returned (if the node knowns less
     * than `nb_nodes` node).
     */
    std::vector<NodeAddress>
    find_node(const UInt160& target_id, uint32_t nb_nodes);

    std::vector<NodeAddress> find_value(const UInt160& key);

    /** Store the specified entry on the node.
     *
     * @param entry the entry to store.
     */
    void store(std::unique_ptr<Entry> entry);

    /** Return the k node that are the closest to `target_id`
     *
     * This method will communicate with other nodes, it is not limited to its
     * own known nodes.
     *
     * @param target_id the targeted node.
     * @return the list of the `k` nodes that are the closest to `target_id`.
     */
    // TODO: eventually, this should probably be private.
    std::vector<NodeAddress> node_lookup(const UInt160& target_id);

    /** Computes the distance to the specified node ID/key. */
    inline UInt160 distance_to(const UInt160& id) const
    {
        return compute_distance(m_addr.id(), id);
    }

    /** Computes the number of nodes that this node can connect to. */
    uint32_t connection_count() const;

    /** Return the node's ID. */
    inline const UInt160& id() const
    {
        return m_addr.id();
    };

    /** Return the node's addreess. */
    inline const NodeAddress& addr() const
    {
        return m_addr;
    };

    inline bool operator==(const Node& other) const
    {
        return m_addr == other.m_addr;
    }

    Node(Node const&) = delete;
    Node& operator=(Node const& x) = delete;
    Node(Node&&) = delete;
    Node& operator=(Node&& x) = delete;

  protected:
    bool register_node(Node* node, bool contacted_us);

    const std::unordered_map<uint32_t, std::list<NodeAddress>>& buckets() const
    {
        return m_buckets;
    }

  private:
    virtual void on_store(const Entry& /* entry */) {}

    /** Refresh the routing table.
     *
     * If the node already exists it is moved in front of the bucket.
     * If the node is new:
     * - bucket not full: insert in front.
     * - bucket full: ping the least recently seen node in the bucket:
     *   - it respond: we move it to the front and ignore `node_id`.
     *   - it doesn't respond: evict it and insert `node_id` in front.
     *
     * @param addr the address of the last node seen
     */
    void refresh_routing_table(const NodeAddress& addr);

    /** Call FIND_NODE on the list of specified nodes.
     *
     * @param nodes_to_query node to query
     * @param target_id      the ID of the target
     * @return the aggregated responses from the queried nodes.
     *
     * @note the returned value is unsorted and may contains duplicates.
     */
    std::vector<NodeAddress> send_find_node(
        const std::vector<NodeAddress>& nodes_to_query,
        const UInt160& target_id,
        std::unordered_set<UInt160>& queried);

    NodeAddress m_addr; /**< The node ID.                          */
    uint32_t m_keysize; /**< Size of the keys (in bits).           */
    uint32_t m_k;       /**< k: system-wide replication parameter. */
    uint32_t m_alpha;   /**< α: system-wide concurrency parameter. */

    /** The list of k-bucket. */
    // TODO: should probably use a heap/priority queue instead of a list here.
    std::unordered_map<uint32_t, std::list<NodeAddress>> m_buckets;
    /** The entries stored on this node. */
    std::vector<std::unique_ptr<Entry>> m_entries;
    /** Module for the inter-node communication. */
    NodeCom m_com_iface;
};

} // namespace dht
} // namespace kad

#include "node.tpp"

#endif
