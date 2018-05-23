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
#include <algorithm>
#include <cassert>
#include <iterator>

#include "com.h"
#include "core.h"
#include "entry.h"
#include "exceptions.h"
#include "kad_conf.h"
#include "node.h"
#include "utils.h"

namespace kad {
namespace dht {

class ByDistanceFrom {
  public:
    explicit ByDistanceFrom(const UInt160& target_id) : m_target(target_id) {}

    /** Compare two Node by their distance to a target Node.
     *
     * @return true if first is closer than second
     */
    bool operator()(const NodeAddress& first, const NodeAddress& second) const
    {
        const UInt160 d1(compute_distance(first.id(), m_target));
        const UInt160 d2(compute_distance(second.id(), m_target));

        return d1 < d2;
    }

  private:
    const UInt160& m_target;
};

template <typename NodeCom>
Node<NodeCom>::Node(NodeAddress addr, const Conf& configuration,
                    const NodeCom& com_iface)
    : m_addr(addr), m_com_iface(com_iface)
{
    m_keysize = configuration.n_bits;
    m_k = configuration.k;
    m_alpha = configuration.alpha;

    // Initialize the k-buckets.
    for (uint32_t i = 0; i < (m_keysize + 1); i++) {
        m_buckets[i] = std::list<NodeAddress>();
    }
}

template <typename NodeCom>
uint32_t Node<NodeCom>::connection_count() const
{
    uint32_t total = 0;

    for (uint32_t i = 0; i < (m_keysize + 1); i++) {
        total += static_cast<uint32_t>(m_buckets.at(i).size());
    }

    return total;
}

template <typename NodeCom>
void Node<NodeCom>::ping(const Node& node)
{
    DHT_LOG(TRACE) << "node " << id() << ": PING(" << node.id() << ')';
    if (m_com_iface.ping(node.m_addr)) {
        refresh_routing_table(node.m_addr);
    }
}

template <typename NodeCom>
std::vector<NodeAddress> Node<NodeCom>::find_node(const UInt160& target_id,
                                                  uint32_t nb_nodes)
{
    DHT_LOG(TRACE) << "node " << id()
                   << ": FIND_NODE(" << target_id << ", " << nb_nodes << ')';

    const UInt160 distance(distance_to(target_id));
    const uint32_t bucket_idx = distance.bit_length();
    std::vector<NodeAddress> closest;

    DHT_VLOG(1) << "distance=" << distance << ", k-bucket=" << bucket_idx;

    // First look in the corresponding k-bucket.
    assert(bucket_idx <= m_buckets.size());
    // FIXME: copy could be avoided here.
    std::list<NodeAddress> k_bucket = m_buckets[bucket_idx];

    // Add the k closest.
    // FIXME: sort by last time seen, not distance (most recent at the tail).
    // Or even better: use a container that enforce this (heap/priority queue).
    k_bucket.sort(ByDistanceFrom(target_id));
    k_bucket.unique();
    safe_copy_n(k_bucket, nb_nodes, closest);

    DHT_VLOG(3) << "got " << closest.size() << '/' << nb_nodes
                << " nodes from the k-bucket " << bucket_idx;

    // If the selected k-bucket doesn't contains enough node, use the others.
    if (closest.size() < nb_nodes) {
        std::list<NodeAddress> all;

        // Find remaining nearest nodes.
        for (uint32_t i = 0; i != m_buckets.size(); ++i) {
            if (bucket_idx == i) {
                continue;
            }
            all.insert(all.end(), m_buckets[i].begin(), m_buckets[i].end());
        }

        // FIXME: see comment on the previous sort/unique.
        all.sort(ByDistanceFrom(target_id));
        all.unique();

        safe_copy_n(all, nb_nodes - closest.size(), closest);
    }

    DHT_VLOG(3) << "found " << closest.size() << " nodes";
    DHT_VLOG(5) << "closest nodes=" << closest;

    return closest;
}

template <typename NodeCom>
std::vector<NodeAddress> Node<NodeCom>::find_value(const UInt160& key)
{
    DHT_LOG(TRACE) << "node " << id() << ": FIND_VALUE(" << key << ')';
    (void)key;
    return {};
}

template <typename NodeCom>
void Node<NodeCom>::store(std::unique_ptr<Entry> entry)
{
    DHT_LOG(TRACE) << "node " << id() << ": STORE(" << entry->key() << ')';
    // TODO real implem: call node_lookup and send STORE to the returned nodes.
    m_entries.push_back(std::move(entry));
    on_store(*m_entries.back());
}

template <typename NodeCom>
void Node<NodeCom>::refresh_routing_table(const NodeAddress& addr)
{
    if (m_addr == addr) {
        throw kad::LogicError("cannot add ourself in our own routing table");
    }

    const uint32_t bit_length = distance_to(addr.id()).bit_length();
    std::list<NodeAddress>& bucket = m_buckets[bit_length];

    const auto it = std::find_if(
        bucket.begin(), bucket.end(), [&addr](const NodeAddress& n) {
            return n == addr;
    });
    // The node is known: move it in front.
    if (it != bucket.end()) {
        DHT_VLOG(5) << id() << ": move " << addr.id() << "in front of the "
                    << bit_length << "-bucket";
        bucket.splice(bucket.begin(), bucket, it);
        return;
    }

    // New node check if there is room for it in the bucket.
    if (bucket.size() < m_k) {
        // Yes, insert in front.
        DHT_VLOG(5) << id() << ": insert " << addr.id() << "in front of the "
                    << bit_length << "-bucket";
        bucket.push_front(addr);
        return;
    }
    // TODO: handle the case when the bucket is full.
    DHT_VLOG(5) << id() << ": ignore " << addr.id() << ", "
                << bit_length << "-bucket is full";
}

template <typename NodeCom>
std::vector<NodeAddress> Node<NodeCom>::send_find_node(
    const std::vector<NodeAddress>& nodes_to_query,
    const UInt160& target_id,
    std::unordered_set<UInt160>& queried)
{
    std::vector<NodeAddress> answers;

    for (auto& remote_node : nodes_to_query) {
        DHT_LOG(TRACE) << "node " << id()
                       << ": send FIND_NODE(" << target_id << ", " <<  m_k
                       << ") to " << remote_node;
        const auto nodes = m_com_iface.find_node(remote_node, target_id, m_k);

        // Don't add ourselves into the list of answers.
        std::copy_if(nodes.cbegin(), nodes.cend(), back_inserter(answers),
                     [this](const NodeAddress& n) { return n.id() != id(); });
        queried.insert(remote_node.id());

        DHT_VLOG(5) << "from " << remote_node
                    << ": nodes(" << nodes.size() << ")=" << nodes;
    }

    std::sort(answers.begin(), answers.end(), ByDistanceFrom(target_id));
    std::unique(answers.begin(), answers.end());

    DHT_VLOG(3) << "got " << answers.size() << " nodes from "
                <<  nodes_to_query.size() << " queried nodes";

    return answers;
}

/* Remove the nodes already queried from `nodes`. */
static inline void remove_queried_nodes(
    std::vector<NodeAddress>& nodes,
    const std::unordered_set<UInt160>& queried)
{
    nodes.erase(
        std::remove_if(
            nodes.begin(),
            nodes.end(),
            [&queried](const NodeAddress& n) {
                return queried.find(n.id()) != queried.end();
            }),
        nodes.end());
}

// Check if `new_nodes` brings nodes closer to the target.
static inline bool has_found_closer_nodes(
    const std::vector<NodeAddress>& cur_nodes,
    const std::vector<NodeAddress>& new_nodes,
    const UInt160& target_id)
{
    // No new node: we can't have gotten closer.
    if (new_nodes.empty()) {
        DHT_VLOG(1) << "found closer nodes: FALSE: no new nodes";
        return false;
    }

    // We had no nodes and now we got some: we have to be closer.
    if (cur_nodes.empty()) {
        DHT_VLOG(1) << "found closer nodes: TRUE: no old nodes";
        return true;
    }

    const UInt160 cur_best(compute_distance(cur_nodes.front().id(), target_id));
    const UInt160 new_best(compute_distance(new_nodes.front().id(), target_id));
    const bool res = new_best < cur_best;

    DHT_VLOG(1) << "found closer nodes: " << (res ? "TRUE" : "FALSE");
    return res;
}

static inline bool is_subset(
    const std::vector<NodeAddress>& nodes,
    const std::unordered_set<UInt160>& set)
{
    for (const auto& node : nodes) {
        if (set.find(node.id()) == set.end()) {
            return false;
        }
    }
    return true;
}

template <typename NodeCom>
std::vector<NodeAddress> Node<NodeCom>::node_lookup(const UInt160& target_id)
{
    std::unordered_set<UInt160> queried({m_addr.id()});
    std::vector<NodeAddress> answers;
    unsigned round = 0;

    DHT_VLOG(1) << "node lookup for " << target_id;

    // Query the α nodes locally known as the closest to the target.
    std::vector<NodeAddress> to_query = find_node(target_id, m_alpha);
    answers = send_find_node(to_query, target_id, queried);

    DHT_VLOG(3) << "INIT: to_query: " << to_query.size()
                << ", queried: " << queried.size()
                << ", answers: " << answers.size();
    DHT_VLOG(5) << "INIT: to_query(" << to_query.size() << ")=" << to_query;
    DHT_VLOG(5) << "INIT: answers("  << answers.size()  << ")=" << answers;
    DHT_VLOG(7) << "INIT: queried("  << queried.size()  << ")=" << queried;

    while (true) {
#define ROUND_VLOG(_level)  DHT_VLOG(_level) << "ROUND " << round << ": "

        DHT_VLOG(3) << "lookup node: ROUND " << ++round;
        std::vector<NodeAddress> k_nodes;

        // From the answers of the previous round, only keep the k-closest.
        safe_copy_n(answers, m_k, k_nodes);
        ROUND_VLOG(5) << "k_nodes(" << k_nodes.size() <<  ")= " << k_nodes;

        // For querying, only keep the new nodes.
        std::vector<NodeAddress> k_new_nodes(k_nodes);
        remove_queried_nodes(k_new_nodes, queried);
        ROUND_VLOG(5) << "k_new_nodes(" << k_new_nodes.size()
                      <<  ")= " << k_new_nodes;

        // Query α nodes from the k-closest.
        to_query.clear();
        safe_copy_n(k_new_nodes, m_alpha, to_query);
        answers = send_find_node(to_query, target_id, queried);
        ROUND_VLOG(3) << "queried alpha nodes";
        ROUND_VLOG(5) << "to_query(" << to_query.size() << ")=" << to_query;
        ROUND_VLOG(5) << "answers("  << answers.size()  << ")=" << answers;
        ROUND_VLOG(7) << "queried("  << queried.size()  << ")=" << queried;

        // We already have queried (and got an answer) from the k closest nodes
        // we know: return them.
        if (to_query.empty() && is_subset(k_nodes, queried)) {
            DHT_VLOG(1) << "found " << k_nodes.size()
                        << " nodes for " << target_id << ": " << k_nodes;
            return k_nodes;
        }

        // If we haven't found a closer node, we query the remaining nodes.
        if (!has_found_closer_nodes(k_nodes, answers, target_id)) {
            const auto rem_begin = k_new_nodes.begin() + m_alpha;

            to_query.clear();
            std::copy(rem_begin, k_new_nodes.end(), back_inserter(to_query));

            const auto new_ans(send_find_node(to_query, target_id, queried));
            answers.insert(answers.end(), new_ans.begin(), new_ans.end());
            ROUND_VLOG(5) << "queried remaining nodes";
            ROUND_VLOG(5) << "to_query(" << to_query.size() << ")=" << to_query;
            ROUND_VLOG(5) << "answers("  << answers.size()  << ")=" << answers;
            ROUND_VLOG(7) << "queried("  << queried.size()  << ")=" << queried;
        }

        ROUND_VLOG(3) << "to_query: " << to_query.size()
                      << ", queried: " << queried.size()
                      << ", k-nodes: " << k_nodes.size()
                      << ", answers: " << answers.size();
#undef ROUND_VLOG
    }

    assert(false);
    return {};
}

} // namespace dht
} // namespace kad
