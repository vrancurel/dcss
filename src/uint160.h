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
#ifndef __KAD_UINT160_H__
#define __KAD_UINT160_H__

#include <array>
#include <cstdint>
#include <random>
#include <string>

namespace kad {

/** A sequence of bytes (up to 160 bits). */
class UInt160 {
  public:
    /** Return a 160-bit integer set to 0. */
    UInt160() : m_limbs({0, 0, 0, 0, 0}) {}

    /** Initialize a 160-bit integer from an unsigned integer.
     *
     * @param n an unsigned value.
     */
    // NOLINTNEXTLINE(google-explicit-constructor)
    UInt160(uint64_t n);

    /** @see UInt160::UInt160(uint64_t) */
    // NOLINTNEXTLINE(google-explicit-constructor)
    UInt160(uint32_t n) : m_limbs({0, 0, 0, 0, n}) {}

    /** @see UInt160::UInt160(uint64_t) */
    // NOLINTNEXTLINE(google-explicit-constructor)
    UInt160(uint16_t n) : UInt160(static_cast<uint32_t>(n)) {}

    /** @see UInt160::UInt160(uint64_t) */
    // NOLINTNEXTLINE(google-explicit-constructor)
    UInt160(uint8_t n) : UInt160(static_cast<uint32_t>(n)) {}

    /** Initialize the UInt160 from an hex string.
     *
     * @param hex a 40-byte hex string.
     *
     * @pre the input string must contains exactly 40 hex char.
     * @throw LogicError — invalid length
     * @throw Exception — invalid character
     */
    explicit UInt160(const std::string& hex);

    /** Generate a random 160-bit integer.
     *
     * @param prng the PRNG to use
     * @return a random 160-bit integer.
     */
    static UInt160 rand(std::mt19937& prng);

    /** Return the hex string representation of the value.
     *
     * @return a 40-char hex string.
     */
    std::string to_string() const;

    /** Return the length (in bit) of the value.
     *
     * @return the position of the highest bit set.
     */
    int bit_length() const;

    /** Return the hashed value of the integer.
     *
     * @return the hash of the value.
     */
    size_t hash() const;

    // Logical operators.
    explicit operator bool() const;
    bool operator!() const;
    friend bool operator&&(const UInt160& a, const UInt160& b);
    friend bool operator||(const UInt160& a, const UInt160& b);

    // Comparison operators.
    friend bool operator==(const UInt160& lhs, const UInt160& rhs);
    friend bool operator!=(const UInt160& lhs, const UInt160& rhs);
    friend bool operator<(const UInt160& lhs, const UInt160& rhs);
    friend bool operator<=(const UInt160& lhs, const UInt160& rhs);
    friend bool operator>(const UInt160& lhs, const UInt160& rhs);
    friend bool operator>=(const UInt160& lhs, const UInt160& rhs);

    // Arithmetic operators.
    UInt160 operator+() const;
    UInt160 operator-() const;
    friend UInt160 operator+(const UInt160& lhs, const UInt160& rhs);
    UInt160& operator+=(const UInt160& rhs);

    // Bitwise operators.
    UInt160 operator~() const;
    UInt160& operator&=(const UInt160& rhs);
    UInt160& operator|=(const UInt160& rhs);
    UInt160& operator^=(const UInt160& rhs);
    friend UInt160 operator&(const UInt160& lhs, const UInt160& rhs);
    friend UInt160 operator|(const UInt160& lhs, const UInt160& rhs);
    friend UInt160 operator^(const UInt160& lhs, const UInt160& rhs);
    UInt160& operator<<=(unsigned shift);
    UInt160& operator>>=(unsigned shift);
    UInt160 operator<<(unsigned shift) const;
    UInt160 operator>>(unsigned shift) const;

    // Output operator.
    friend std::ostream& operator<<(std::ostream& os, const UInt160& n);

    ~UInt160() = default;
    UInt160(UInt160 const&) = default;
    UInt160& operator=(UInt160 const& x) = default;
    UInt160(UInt160&&) = default;
    UInt160& operator=(UInt160&& x) = default;

  private:
    /* Size of the hex string that can represent an 160-bit integer. */
    static const int HEX_SIZE = 160 / 4;

    /* "Limbs" of the integer, in "big-endian".
     *
     * (m_limbs[0] ([4]) contains the most (least) significant bits).
     */
    std::array<uint32_t, 5> m_limbs;
};

} // namespace kad

// std::hash implementation for UInt160.
namespace std {
template <>
struct hash<kad::UInt160> {
    size_t operator()(const kad::UInt160& n) const
    {
        return n.hash();
    }
};
} // namespace std

#endif
