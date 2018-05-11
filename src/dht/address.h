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
#ifndef __KAD_DHT_ADDRESS_H__
#define __KAD_DHT_ADDRESS_H__

#include <cstdint>

#include "uint160.h"

namespace kad {
namespace dht {

// TODO: just a placeholder for now, should be implemented later.
class IpAddress {
  public:
    explicit IpAddress(const std::string& ip)
    {
        // TODO: parse the IP using `inet_pton`
        (void)ip;
    }
};

class NodeAddress {
  public:
    NodeAddress(UInt160 id, const std::string& ip, uint16_t port)
        : m_id(id), m_ip(ip), m_port(port)
    {
    }

    /** Return the node's ID. */
    inline const UInt160& id() const
    {
        return m_id;
    };

    /** Return the node's IP. */
    inline const IpAddress& ip() const
    {
        return m_ip;
    };

    /** Return the node's port. */
    inline uint16_t port() const
    {
        return m_port;
    };

    inline bool operator==(const NodeAddress& other) const
    {
        return m_id == other.m_id;
    }

  private:
    UInt160 m_id;
    IpAddress m_ip;
    uint16_t m_port;
};

} // namespace dht
} // namespace kad

// Implementing std::hash for dht::NodeAddress.
namespace std {
template <>
struct hash<kad::dht::NodeAddress> {
    size_t operator()(const kad::dht::NodeAddress& addr) const
    {
        return hash<kad::UInt160>()(addr.id());
    }
};
} // namespace std

#endif
