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
#ifndef __DCSS_DHT_ENTRY_H__
#define __DCSS_DHT_ENTRY_H__

#include "uint160.h"

namespace dcss {
namespace dht {

/** A DHT entry, identified by its key. */
class Entry {
  public:
    /** Create a new DHT item identified by `id` and stored on `node`.
     *
     * @param key   a unique key that identify the entry
     * @param value the entry payload
     */
    Entry(UInt160 key, std::string value)
        : m_key(key), m_value(std::move(value))
    {
    }
    virtual ~Entry() = default;

    /** Return the entry key. */
    inline const UInt160& key() const
    {
        return m_key;
    };

    /** Return the entry value. */
    inline const std::string& value() const
    {
        return m_value;
    };

    Entry(Entry const&) = delete;
    Entry& operator=(Entry const& x) = delete;
    Entry(Entry&&) = delete;
    Entry& operator=(Entry&& x) = delete;

  private:
    UInt160 m_key;       /**< Key of the entry in the DHT.   */
    std::string m_value; /**< Value of the entry in the DHT. */
};

} // namespace dht
} // namespace dcss

#endif
