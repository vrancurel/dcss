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

// Return a 160-bit zero.
static inline const UInt160& one()
{
    static const UInt160 one(1u);
    return one;
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

// Binary version of the famous long division algorithm.
// Source: https://en.wikipedia.org/wiki/Division_algorithm
static inline std::pair<UInt160, UInt160>
divmod(const UInt160& lhs, const UInt160& rhs)
{
    UInt160 quot{};
    UInt160 rem{};

    for (unsigned i = lhs.bit_length(); i-- > 0;) {
        rem <<= 1u;
        quot <<= 1u;

        if (!!(lhs & (one() << i))) {
            ++rem;
        }

        if (rem >= rhs) {
            rem -= rhs;
            ++quot;
        }
    }
    return std::make_pair(quot, rem);
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
    return *this != zero();
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

/** Generate implementation for the relational operators.
 *
 * The task is delegated to the underlying std::array.
 */
#define IMPL_ORD(op)                                                           \
    bool operator op(const UInt160& lhs, const UInt160& rhs)                   \
    {                                                                          \
        return lhs.m_limbs op rhs.m_limbs;                                     \
    }

IMPL_ORD(==)
IMPL_ORD(!=)
IMPL_ORD(<)
IMPL_ORD(<=)
IMPL_ORD(>)
IMPL_ORD(>=)

#undef IMPL_ORD

UInt160 UInt160::operator+() const
{
    return *this;
}

// Return the inverse of this.
//
// In a ring the inverse of `a` defined as: a + (-a) <=> (-a) + a <=> 0
// Which correspond to the bitwise NOT + 1 in a ring mod 2^n.
UInt160 UInt160::operator-() const
{
    return ~(*this) + one();
}

UInt160 operator+(const UInt160& lhs, const UInt160& rhs)
{
    UInt160 tmp(lhs);
    tmp += rhs;
    return tmp;
}

UInt160 operator-(const UInt160& lhs, const UInt160& rhs)
{
    UInt160 tmp(lhs);
    tmp -= rhs;
    return tmp;
}

// Based on the algorithm M from The Art of Computer Programming, vol. 2 by
// Donald Knuth.
UInt160 operator*(const UInt160& lhs, const UInt160& rhs)
{
    // Shortcut for simple cases.
    if (lhs == zero() || rhs == zero()) {
        return zero();
    }
    if (lhs == one()) {
        return rhs;
    }
    if (rhs == one()) {
        return lhs;
    }

    // Main algorithm.
    UInt160 res{};
    uint32_t carry = 0;
    size_t offset = 0;

    for (size_t j = rhs.m_limbs.size(); j-- > 0;) {
        for (size_t i = lhs.m_limbs.size(); i-- > offset;) {
            const size_t dst = i - offset;
            const uint64_t sum = static_cast<uint64_t>(lhs.m_limbs[i])
                                     * static_cast<uint64_t>(rhs.m_limbs[j])
                                 + static_cast<uint64_t>(res.m_limbs[dst])
                                 + carry;

            res.m_limbs[dst] = static_cast<uint32_t>(sum);
            carry = static_cast<uint32_t>(sum >> 32u);
        }
        carry = 0;
        ++offset;
    }

    return res;
}

UInt160 operator/(const UInt160& lhs, const UInt160& rhs)
{
    // Shortcut for simple cases.
    if (rhs == zero()) {
        throw DomainError("division by zero");
    }
    if (lhs == zero() || lhs < rhs) {
        return zero();
    }
    if (lhs == rhs) {
        return one();
    }
    if (rhs == one()) {
        return lhs;
    }

    return divmod(lhs, rhs).first;
}

UInt160 operator%(const UInt160& lhs, const UInt160& rhs)
{
    // Shortcut for simple cases.
    if (rhs == zero()) {
        throw DomainError("division by zero");
    }
    if (lhs == zero() || rhs == one() || lhs == rhs) {
        return zero();
    }

    return divmod(lhs, rhs).second;
}

// Based on the algorithm A from The Art of Computer Programming, vol. 2 by
// Donald Knuth.
UInt160& UInt160::operator+=(const UInt160& rhs)
{
    uint32_t carry = 0;
    for (size_t i = m_limbs.size(); i-- > 0;) {
        const uint64_t sum = static_cast<uint64_t>(m_limbs[i])
                             + static_cast<uint64_t>(rhs.m_limbs[i]) + carry;

        m_limbs[i] = static_cast<uint32_t>(sum);
        carry = static_cast<uint32_t>(sum >> 32u);
    }
    return *this;
}

// In a ring, the subtraction is defined as: a - b <=> a + (-b)
UInt160& UInt160::operator-=(const UInt160& rhs)
{
    return *this += -rhs;
}

UInt160& UInt160::operator*=(const UInt160& rhs)
{
    *this = *this * rhs;
    return *this;
}

UInt160& UInt160::operator/=(const UInt160& rhs)
{
    *this = *this / rhs;
    return *this;
}

UInt160& UInt160::operator%=(const UInt160& rhs)
{
    *this = *this % rhs;
    return *this;
}

UInt160& UInt160::operator++()
{
    return *this += one();
}

UInt160& UInt160::operator--()
{
    return *this -= one();
}

const UInt160 UInt160::operator++(int)
{
    const UInt160 tmp(*this);

    *this += one();
    return tmp;
}

const UInt160 UInt160::operator--(int)
{
    const UInt160 tmp(*this);

    *this -= one();
    return tmp;
}

UInt160 UInt160::operator~() const
{
    UInt160 res(*this);

    for (auto& limb : res.m_limbs) {
        limb = ~limb;
    }

    return res;
}

/** Generate implemention for the bitwise logical binary operators.
 *
 * To do so, the operation is applied limb-wise.
 */
#define IMPL_BITWISE_LOGICAL(_op)                                              \
    UInt160& UInt160::operator _op##=(const UInt160& rhs)                      \
    {                                                                          \
        for (size_t i = 0; i != m_limbs.size(); ++i) {                         \
            m_limbs[i] _op## = rhs.m_limbs[i];                                 \
        }                                                                      \
                                                                               \
        return *this;                                                          \
    }                                                                          \
                                                                               \
    UInt160 operator _op(const UInt160& lhs, const UInt160& rhs)               \
    {                                                                          \
        UInt160 res(lhs);                                                      \
                                                                               \
        return res _op## = rhs;                                                \
    }

IMPL_BITWISE_LOGICAL(&)
IMPL_BITWISE_LOGICAL(|)
IMPL_BITWISE_LOGICAL (^)

#undef IMPL_BITWISE_LOGICAL

UInt160& UInt160::operator<<=(unsigned shift)
{
    if (shift == 0) {
        return *this;
    }
    if (shift >= 160) {
        *this = zero();
        return *this;
    }

    // number of block between the source and destination.
    size_t gap = shift / 32;
    // number of block to shift (the others are zeroed).
    size_t to_shift = m_limbs.size() - gap;
    shift %= 32;
    for (size_t i = 0; i != to_shift - 1; ++i) {
        const size_t src = i + gap;
        const uint32_t hi = m_limbs[src] << shift;
        const uint32_t lo = shift != 0u ? m_limbs[src + 1] >> (32 - shift) : 0;
        m_limbs[i] = hi | lo;
    }
    // last block (just before the zeroed blocks).
    m_limbs[to_shift - 1] = m_limbs[to_shift - 1 + gap] << shift;
    // set trailing blocks to zero.
    std::fill(m_limbs.begin() + to_shift, m_limbs.end(), 0u);

    return *this;
}

UInt160& UInt160::operator>>=(unsigned shift)
{
    if (shift == 0) {
        return *this;
    }
    if (shift >= 160) {
        *this = zero();
        return *this;
    }

    // number of block between the source and destination.
    size_t gap = shift / 32;
    // number of block to shift (the others are zeroed).
    size_t to_shift = gap;
    shift %= 32;
    for (size_t i = m_limbs.size() - 1; i != to_shift; --i) {
        const size_t src = i - gap;
        const uint32_t lo = m_limbs[src] >> shift;
        const uint32_t hi = shift != 0u ? m_limbs[src - 1] << (32 - shift) : 0;
        m_limbs[i] = hi | lo;
    }
    // last block (just before the zeroed blocks).
    m_limbs[to_shift] = m_limbs[to_shift - gap] >> shift;
    // set trailing blocks to zero.
    std::fill(m_limbs.begin(), m_limbs.begin() + to_shift, 0u);

    return *this;
}

UInt160 UInt160::operator<<(unsigned shift) const
{
    UInt160 tmp(*this);

    tmp <<= shift;
    return tmp;
}

UInt160 UInt160::operator>>(unsigned shift) const
{
    UInt160 tmp(*this);

    tmp >>= shift;
    return tmp;
}

std::ostream& operator<<(std::ostream& os, const UInt160& n)
{
    // TODO: handle formatter such as std::dec, std::hex and std::oct?
    return os << n.to_string();
}

} // namespace kad
