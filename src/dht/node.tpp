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
    if (m_com_iface.ping(node.m_addr)) {
        refresh_routing_table(node.m_addr);
    }
}

template <typename NodeCom>
std::vector<NodeAddress> Node<NodeCom>::find_node(const UInt160& target_id,
                                                  uint32_t nb_nodes)
{
    const UInt160 distance(distance_to(target_id));
    const uint32_t bucket_idx = distance.bit_length();
    std::vector<NodeAddress> closest;

    // First look in the corresponding k-bucket.
    assert(bucket_idx <= m_buckets.size());
    // FIXME: copy could be avoided here.
    std::list<NodeAddress> k_bucket = m_buckets[bucket_idx];

    // Add the k closest.
    // FIXME: sort by last time seen, not distance (most recent at the tail).
    // Or even better: use a container that enforce this (heap/priority queue).
    k_bucket.sort(ByDistanceFrom(target_id));
    k_bucket.unique();
    std::copy_n(k_bucket.begin(), nb_nodes, std::back_inserter(closest));

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

        std::copy_n(
            all.begin(),
            nb_nodes - closest.size(),
            std::back_inserter(closest));
    }

    return closest;
}

template <typename NodeCom>
std::vector<NodeAddress> Node<NodeCom>::find_value(const UInt160& key)
{
    (void)key;
    return {};
}

template <typename NodeCom>
void Node<NodeCom>::store(std::unique_ptr<Entry> entry)
{
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
        bucket.splice(bucket.begin(), bucket, it);
        return;
    }

    // New node check if there is room for it in the bucket.
    if (bucket.size() < m_k) {
        // Yes, insert in front.
        bucket.push_front(addr);
    }
    // TODO: handle the case when the bucket is full.
}

template <typename NodeCom>
std::vector<NodeAddress> Node<NodeCom>::send_find_node(
    const std::vector<NodeAddress>& nodes_to_query,
    const UInt160& target_id,
    std::unordered_set<UInt160>& queried)
{
    std::vector<NodeAddress> answers;

    for (auto& remote_node : nodes_to_query) {
        const auto nodes = m_com_iface.find_node(remote_node, target_id, m_k);

        // Don't add ourselves into the list of answers.
        std::copy_if(nodes.cbegin(), nodes.cend(), back_inserter(answers),
                     [this](const NodeAddress& n) { return n.id() != id(); });
        queried.insert(remote_node.id());
    }

    std::sort(answers.begin(), answers.end(), ByDistanceFrom(target_id));
    std::unique(answers.begin(), answers.end());

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
        return false;
    }

    // We had no nodes and now we got some: we have to be closer.
    if (cur_nodes.empty()) {
        return true;
    }

    const UInt160 cur_best(compute_distance(cur_nodes.front().id(), target_id));
    const UInt160 new_best(compute_distance(new_nodes.front().id(), target_id));

    return new_best < cur_best;
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

    // Query the α nodes locally known as the closest to the target.
    std::vector<NodeAddress> to_query = find_node(target_id, m_alpha);
    answers = send_find_node(to_query, target_id, queried);

    while (true) {
        std::vector<NodeAddress> k_nodes;

        // From the answers of the previous round, only keep the k-closest.
        std::copy_n(answers.begin(), m_k, back_inserter(k_nodes));

        // For querying, only keep the new nodes.
        std::vector<NodeAddress> k_new_nodes(k_nodes);
        remove_queried_nodes(k_new_nodes, queried);

        // Query α nodes from the k-closest.
        to_query.clear();
        std::copy_n(k_new_nodes.begin(), m_alpha, back_inserter(to_query));
        answers = send_find_node(to_query, target_id, queried);

        // We already have queried (and got an answer) from the k closest nodes
        // we know: return them.
        if (to_query.empty() && is_subset(k_nodes, queried)) {
            return k_nodes;
        }

        // If we haven't found a closer node, we query the remaining nodes.
        if (!has_found_closer_nodes(k_nodes, answers, target_id)) {
            const auto rem_begin = k_new_nodes.begin() + m_alpha;

            to_query.clear();
            std::copy(rem_begin, k_new_nodes.end(), back_inserter(to_query));

            const auto new_ans(send_find_node(to_query, target_id, queried));
            answers.insert(answers.end(), new_ans.begin(), new_ans.end());
        }
    }

    assert(false);
    return {};
}

} // namespace dht
} // namespace kad
