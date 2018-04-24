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
#include "exceptions.h"
#include "uint160.h"

namespace kad {

// Return a 160-bit zero.
static inline const UInt160& zero()
{
    static const UInt160 zero(0u);
    return zero;
}

// Return the integral value of an hex digit.
static inline uint32_t decode_hex_char(char hex)
{
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    }
    if (hex >= 'a' && hex <= 'z') {
        return hex - 'a' + 0xA;
    }
    if (hex >= 'A' && hex <= 'Z') {
        return hex - 'A' + 0xA;
    }
    throw Exception("invalid hex string: bad character");
}

// Return the position of the last bit set in a 32-bit word.
//
// From: https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
//
// Based on:
// "Using de Bruijn Sequences to Index a 1 in a Computer Word", 1998,
// by Charles E. Leiserson , Harald Prokop , Keith H. Randall.
// http://supertech.csail.mit.edu/papers/debruijn.pdf
static inline uint32_t fls(uint32_t n)
{
    static const int lookup[32] = {0,  9,  1,  10, 13, 21, 2,  29, 11, 14, 16,
                                   18, 22, 25, 3,  30, 8,  12, 20, 28, 15, 17,
                                   24, 7,  19, 27, 23, 6,  26, 5,  4,  31};

    // First, round to the next power of two - 1 (i.e 37 becomes 63).
    n |= n >> 1u;
    n |= n >> 2u;
    n |= n >> 4u;
    n |= n >> 8u;
    n |= n >> 16u;

    return n != 0u ? lookup[(n * 0x07C4ACDD) >> 27u] + 1 : n;
}

UInt160::UInt160(uint64_t n) : UInt160()
{
    m_limbs[m_limbs.size() - 1] = static_cast<uint32_t>(n >> 0u);
    m_limbs[m_limbs.size() - 2] = static_cast<uint32_t>(n >> 32u);
}

UInt160::UInt160(const std::string& hex) : UInt160()
{
    // TODO: be less strict?
    if (hex.size() != HEX_SIZE) {
        throw LogicError("invalid hex string: bad length");
    }
    // Iterate by chunk of 8 char: 32-bit is represented by 8 hex char.
    for (std::string::size_type i = 0; i != hex.size(); i += 8) {
        m_limbs[i / 8] = (decode_hex_char(hex[i + 0]) << 28u)
                         | (decode_hex_char(hex[i + 1]) << 24u)
                         | (decode_hex_char(hex[i + 2]) << 20u)
                         | (decode_hex_char(hex[i + 3]) << 16u)
                         | (decode_hex_char(hex[i + 4]) << 12u)
                         | (decode_hex_char(hex[i + 5]) << 8u)
                         | (decode_hex_char(hex[i + 6]) << 4u)
                         | (decode_hex_char(hex[i + 7]) << 0u);
    }
}

UInt160 UInt160::rand(std::mt19937& prng)
{
    UInt160 n{};

    for (auto& limb : n.m_limbs) {
        limb = static_cast<uint32_t>(prng());
    }

    return n;
}

std::string UInt160::to_string() const
{
    static const char charset[] = "0123456789abcdef";
    std::string hex;

    hex.reserve(HEX_SIZE);
    for (const auto& limb : m_limbs) {
        hex.push_back(charset[(limb >> 28u) & 0xFu]);
        hex.push_back(charset[(limb >> 24u) & 0xFu]);
        hex.push_back(charset[(limb >> 20u) & 0xFu]);
        hex.push_back(charset[(limb >> 16u) & 0xFu]);
        hex.push_back(charset[(limb >> 12u) & 0xFu]);
        hex.push_back(charset[(limb >> 8u) & 0xFu]);
        hex.push_back(charset[(limb >> 4u) & 0xFu]);
        hex.push_back(charset[(limb >> 0u) & 0xFu]);
    }
    return hex;
}

int UInt160::bit_length() const
{
    int offset = 128;

    for (size_t i = 0; i != m_limbs.size(); ++i) {
        const int pos = fls(m_limbs[i]);
        if (pos != 0) {
            return offset + pos;
        }
        offset -= 32;
    }
    return 0;
}

size_t UInt160::hash() const
{
    size_t h = 0;

    for (const auto& limb : m_limbs) {
        // From boost::hash
        h ^= limb + 0x9e3779b9u + (h << 6u) + (h >> 2u);
    }
    return h;
}

UInt160::operator bool() const
{
    return !(*this == zero());
}

bool UInt160::operator!() const
{
    return !bool(*this);
}

bool operator&&(const UInt160& a, const UInt160& b)
{
    return bool(a) && bool(b);
}

bool operator||(const UInt160& a, const UInt160& b)
{
    return bool(a) || bool(b);
}

bool operator==(const UInt160& lhs, const UInt160& rhs)
{
    return lhs.m_limbs == rhs.m_limbs;
}

std::ostream& operator<<(std::ostream& os, const UInt160& n)
{
    // TODO: handle formatter such as std::dec, std::hex and std::oct?
    return os << n.to_string();
}

} // namespace kad
