// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_BIGNUM_H
#define BITCOIN_BIGNUM_H

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <openssl/bn.h>

using int64 = long long;           // NOLINT(google-runtime-int)
using uint64 = unsigned long long; // NOLINT(google-runtime-int)

/** Errors thrown by the bignum class. */
class bignum_error : public std::runtime_error {
  public:
    explicit bignum_error(const std::string& str) : std::runtime_error(str) {}
};

/** RAII encapsulated BN_CTX (OpenSSL bignum context). */
class CAutoBN_CTX {
  protected:
    BN_CTX* pctx;

    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    BN_CTX* operator=(BN_CTX* pnew)
    {
        return pctx = pnew;
    }

  public:
    CAutoBN_CTX()
    {
        pctx = BN_CTX_new();
        if (pctx == nullptr) {
            throw bignum_error("CAutoBN_CTX : BN_CTX_new() returned NULL");
        }
    }

    ~CAutoBN_CTX()
    {
        if (pctx != nullptr) {
            BN_CTX_free(pctx);
        }
    }

    operator BN_CTX*() // NOLINT(google-explicit-constructor)
    {
        return pctx;
    }

    BN_CTX& operator*()
    {
        return *pctx;
    }

    BN_CTX** operator&()
    {
        return &pctx;
    }

    bool operator!()
    {
        return (pctx == nullptr);
    }

    CAutoBN_CTX(CAutoBN_CTX const&) = delete;
    CAutoBN_CTX& operator=(CAutoBN_CTX const& x) = delete;
    CAutoBN_CTX(CAutoBN_CTX&&) = delete;
    CAutoBN_CTX& operator=(CAutoBN_CTX&& x) = delete;
};

/** C++ wrapper for BIGNUM (OpenSSL bignum). */
class CBigNum {
    BIGNUM* bn;

  public:
    CBigNum() : bn(BN_new()) {}

    CBigNum(const CBigNum& b) : CBigNum()
    {
        if (BN_copy(bn, b.bn) == nullptr) {
            BN_clear_free(bn);
            throw bignum_error(
                "CBigNum::CBigNum(const CBigNum&) : BN_copy failed");
        }
    }

    CBigNum(CBigNum&& b) noexcept : bn(b.bn)
    {
        b.bn = nullptr;
    }

    CBigNum& operator=(const CBigNum& b)
    {
        if (BN_copy(bn, b.bn) == nullptr) {
            throw bignum_error("CBigNum::operator= : BN_copy failed");
        }
        return *this;
    }

    CBigNum& operator=(CBigNum&& b) noexcept
    {
        std::swap(bn, b.bn);
        return *this;
    }

    ~CBigNum()
    {
        BN_clear_free(bn);
    }

    // CBigNum(char n) is not portable.  Use 'signed char' or 'unsigned char'.
    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(signed char n) : CBigNum()
    {
        if (n >= 0) {
            setulong(n);
        } else {
            setint64(n);
        }
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(short n) : CBigNum()
    {
        if (n >= 0) {
            setulong(n);
        } else {
            setint64(n);
        }
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(int n) : CBigNum()
    {
        if (n >= 0) {
            setulong(n);
        } else {
            setint64(n);
        }
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(long n) : CBigNum()
    {
        if (n >= 0) {
            setulong(n);
        } else {
            setint64(n);
        }
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(int64 n) : CBigNum()
    {
        setint64(n);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(unsigned char n) : CBigNum()
    {
        setulong(n);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(unsigned short n) : CBigNum()
    {
        setulong(n);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(unsigned int n) : CBigNum()
    {
        setulong(n);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(unsigned long n) : CBigNum()
    {
        setulong(n);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    CBigNum(uint64 n) : CBigNum()
    {
        setuint64(n);
    }

    explicit CBigNum(const std::vector<unsigned char>& vch) : CBigNum()
    {
        setvch(vch);
    }

    void setulong(unsigned long n) // NOLINT(google-runtime-int)
    {
        if (BN_set_word(bn, n) == 0) {
            throw bignum_error(
                "CBigNum conversion from unsigned long : BN_set_word failed");
        }
    }

    unsigned long getulong() const // NOLINT(google-runtime-int)
    {
        return BN_get_word(bn);
    }

    void setint64(int64 sn)
    {
        unsigned char pch[sizeof(sn) + 6];
        unsigned char* p = pch + 4;
        bool fNegative;
        uint64 n;

        if (sn < static_cast<int64>(0)) {
            // Since the minimum signed integer cannot be represented as
            // positive so long as its type is signed, and it's not well-defined
            // what happens if you make it unsigned before negating it, we
            // instead increment the negative integer by 1, convert it, then
            // increment the (now positive) unsigned integer by 1 to compensate
            n = -(sn + 1);
            ++n;
            fNegative = true;
        } else {
            n = sn;
            fNegative = false;
        }

        bool fLeadingZeroes = true;
        for (int i = 0; i < 8; i++) {
            unsigned char c = (n >> 56) & 0xff;
            n <<= 8;
            if (fLeadingZeroes) {
                if (c == 0) {
                    continue;
                }
                if ((c & 0x80) != 0) {
                    *p++ = (fNegative ? 0x80 : 0);
                } else if (fNegative) {
                    c |= 0x80;
                }
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        uint64 nSize = p - (pch + 4);
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize)&0xff;
        BN_mpi2bn(pch, static_cast<int>(p - pch), bn);
    }

    void setuint64(uint64 n)
    {
        unsigned char pch[sizeof(n) + 6];
        unsigned char* p = pch + 4;
        bool fLeadingZeroes = true;
        for (int i = 0; i < 8; i++) {
            unsigned char c = (n >> 56) & 0xff;
            n <<= 8;
            if (fLeadingZeroes) {
                if (c == 0) {
                    continue;
                }
                if ((c & 0x80) != 0) {
                    *p++ = 0;
                }
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        uint64 nSize = p - (pch + 4);
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize)&0xff;
        BN_mpi2bn(pch, static_cast<int>(p - pch), bn);
    }

    void setvch(const std::vector<unsigned char>& vch)
    {
        std::vector<unsigned char> vch2(vch.size() + 4);
        uint64 nSize = vch.size();
        // BIGNUM's byte stream format expects 4 bytes of
        // big endian size data info at the front
        vch2[0] = (nSize >> 24) & 0xff;
        vch2[1] = (nSize >> 16) & 0xff;
        vch2[2] = (nSize >> 8) & 0xff;
        vch2[3] = (nSize >> 0) & 0xff;
        // swap data to big endian
        reverse_copy(vch.begin(), vch.end(), vch2.begin() + 4);
        BN_mpi2bn(&vch2[0], static_cast<int>(vch2.size()), bn);
    }

    std::vector<unsigned char> getvch() const
    {
        unsigned int nSize = BN_bn2mpi(bn, nullptr);
        if (nSize <= 4) {
            return std::vector<unsigned char>();
        }
        std::vector<unsigned char> vch(nSize);
        BN_bn2mpi(bn, &vch[0]);
        vch.erase(vch.begin(), vch.begin() + 4);
        reverse(vch.begin(), vch.end());
        return vch;
    }

    // The "compact" format is a representation of a whole
    // number N using an unsigned 32bit number similar to a
    // floating point format.
    // The most significant 8 bits are the unsigned exponent of base 256.
    // This exponent can be thought of as "number of bytes of N".
    // The lower 23 bits are the mantissa.
    // Bit number 24 (0x800000) represents the sign of N.
    // N = (-1^sign) * mantissa * 256^(exponent-3)
    //
    // Satoshi's original implementation used BN_bn2mpi() and BN_mpi2bn().
    // MPI uses the most significant bit of the first byte as sign.
    // Thus 0x1234560000 is compact (0x05123456)
    // and  0xc0de000000 is compact (0x0600c0de)
    // (0x05c0de00) would be -0x40de000000
    //
    // Bitcoin only uses this "compact" format for encoding difficulty
    // targets, which are unsigned 256bit quantities.  Thus, all the
    // complexities of the sign bit and using base 256 are probably an
    // implementation accident.
    //
    // This implementation directly uses shifts instead of going
    // through an intermediate MPI representation.
    CBigNum& SetCompact(unsigned int nCompact)
    {
        unsigned int nSize = nCompact >> 24;
        bool fNegative = (nCompact & 0x00800000) != 0;
        unsigned int nWord = nCompact & 0x007fffff;
        if (nSize <= 3) {
            nWord >>= 8 * (3 - nSize);
            BN_set_word(bn, nWord);
        } else {
            BN_set_word(bn, nWord);
            BN_lshift(bn, bn, 8 * (nSize - 3));
        }
        BN_set_negative(bn, static_cast<int>(fNegative));
        return *this;
    }

    uint64 GetCompact() const
    {
        uint64 nSize = BN_num_bytes(bn);
        uint64 nCompact = 0;
        if (nSize <= 3) {
            nCompact = BN_get_word(bn) << 8 * (3 - nSize);
        } else {
            CBigNum b;
            BN_rshift(b.bn, bn, 8 * (static_cast<int>(nSize) - 3));
            nCompact = BN_get_word(b.bn);
        }
        // The 0x00800000 bit denotes the sign.
        // Thus, if it is already set, divide the mantissa by 256 and increase
        // the exponent.
        if ((nCompact & 0x00800000) != 0u) {
            nCompact >>= 8;
            nSize++;
        }
        nCompact |= nSize << 24;
        nCompact |= (BN_is_negative(bn) != 0 ? 0x00800000 : 0);
        return nCompact;
    }

    void SetHex(const std::string& str)
    {
        // skip 0x
        const char* psz = str.c_str();
        while (isspace(*psz) != 0) {
            psz++;
        }
        bool fNegative = false;
        if (*psz == '-') {
            fNegative = true;
            psz++;
        }
        if (psz[0] == '0' && tolower(psz[1]) == 'x') {
            psz += 2;
        }
        while (isspace(*psz) != 0) {
            psz++;
        }

        // hex string to bignum
        static const signed char phexdigit[256] = {
            0, 0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 1,   2,   3,   4,   5,   6,   7, 8, 9, 0, 0, 0, 0, 0, 0,
            0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        *this = 0;
        while (isxdigit(*psz) != 0) {
            *this <<= 4;
            int n = phexdigit[static_cast<unsigned char>(*psz++)];
            *this += n;
        }
        if (fNegative) {
            *this = 0 - *this;
        }
    }

    std::string ToString(int nBase = 10) const
    {
        CAutoBN_CTX pctx;
        CBigNum bnBase(nBase);
        CBigNum bn0(0);
        std::string str;
        CBigNum bn = *this;
        BN_set_negative(bn.bn, 0);
        CBigNum dv;
        CBigNum rem;
        if (BN_cmp(bn.bn, bn0.bn) == 0) {
            return "0";
        }
        while (BN_cmp(bn.bn, bn0.bn) > 0) {
            if (BN_div(dv.bn, rem.bn, bn.bn, bnBase.bn, pctx) == 0) {
                throw bignum_error("CBigNum::ToString() : BN_div failed");
            }
            bn = dv;
            uint64 c = rem.getulong();
            str += "0123456789abcdef"[c];
        }
        if (BN_is_negative(this->bn) != 0) {
            str += "-";
        }
        reverse(str.begin(), str.end());
        return str;
    }

    std::string GetHex() const
    {
        return ToString(16);
    }

    /**
     * Returns the number of bits in a CBigNum
     *
     * This is used by Kademlia to figure out which bucket nodes go in
     *
     * bit_length( 0 ) returns 0
     * bit_length( 1 ) returns 1
     * bit_length( 2 ) returns 2
     * bit_length( 3 ) returns 2
     * bit_length( 4 ) returns 3
     * bit_length( 5 ) returns 3
     * bit_length( 6 ) returns 3
     * bit_length( 7 ) returns 3
     *
     * ... and so on
     *
     * @return The number of bits
     */
    inline int bit_length()
    {
        return BN_num_bits(bn);
    }

    inline void Rand(int n_bits)
    {
        CBigNum range(0);
        BN_set_bit(range.bn, n_bits);
        BN_rand_range(bn, range.bn);
    }

    bool operator!() const
    {
        return BN_is_zero(bn) != 0;
    }

    CBigNum& operator+=(const CBigNum& b)
    {
        if (BN_add(bn, bn, b.bn) == 0) {
            throw bignum_error("CBigNum::operator+= : BN_add failed");
        }
        return *this;
    }

    CBigNum& operator-=(const CBigNum& b)
    {
        *this = *this - b;
        return *this;
    }

    CBigNum& operator*=(const CBigNum& b)
    {
        CAutoBN_CTX pctx;
        if (BN_mul(bn, bn, b.bn, pctx) == 0) {
            throw bignum_error("CBigNum::operator*= : BN_mul failed");
        }
        return *this;
    }

    CBigNum& operator/=(const CBigNum& b)
    {
        *this = *this / b;
        return *this;
    }

    CBigNum& operator%=(const CBigNum& b)
    {
        *this = *this % b;
        return *this;
    }

    CBigNum& operator<<=(unsigned int shift)
    {
        if (BN_lshift(bn, bn, shift) == 0) {
            throw bignum_error("CBigNum:operator<<= : BN_lshift failed");
        }
        return *this;
    }

    CBigNum& operator>>=(unsigned int shift)
    {
        // Note: BN_rshift segfaults on 64-bit if 2^shift is greater than the
        // number
        //   if built on ubuntu 9.04 or 9.10, probably depends on version of
        //   OpenSSL
        CBigNum a(1);
        a <<= shift;
        if (BN_cmp(a.bn, bn) > 0) {
            *this = 0;
            return *this;
        }

        if (BN_rshift(bn, bn, shift) == 0) {
            throw bignum_error("CBigNum:operator>>= : BN_rshift failed");
        }
        return *this;
    }

    CBigNum& operator++()
    {
        // prefix operator
        if (BN_add(bn, bn, BN_value_one()) == 0) {
            throw bignum_error("CBigNum::operator++ : BN_add failed");
        }
        return *this;
    }

    const CBigNum operator++(int)
    {
        // postfix operator
        const CBigNum ret = *this;
        ++(*this);
        return ret;
    }

    CBigNum& operator--()
    {
        // prefix operator
        CBigNum r;
        if (BN_sub(r.bn, bn, BN_value_one()) == 0) {
            throw bignum_error("CBigNum::operator-- : BN_sub failed");
        }
        *this = r;
        return *this;
    }

    const CBigNum operator--(int)
    {
        // postfix operator
        const CBigNum ret = *this;
        --(*this);
        return ret;
    }

    friend inline const CBigNum operator-(const CBigNum& a);
    friend inline const CBigNum operator+(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator-(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator/(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator*(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum operator%(const CBigNum& a, const CBigNum& b);
    friend inline const CBigNum
    operator<<(const CBigNum& a, unsigned int shift);
    friend inline const CBigNum
    operator>>(const CBigNum& a, unsigned int shift);
    friend inline const CBigNum operator^(const CBigNum& a, const CBigNum& b);
    friend inline bool operator==(const CBigNum& a, const CBigNum& b);
    friend inline bool operator!=(const CBigNum& a, const CBigNum& b);
    friend inline bool operator<=(const CBigNum& a, const CBigNum& b);
    friend inline bool operator>=(const CBigNum& a, const CBigNum& b);
    friend inline bool operator<(const CBigNum& a, const CBigNum& b);
    friend inline bool operator>(const CBigNum& a, const CBigNum& b);
};

inline const CBigNum operator+(const CBigNum& a, const CBigNum& b)
{
    CBigNum r;
    if (BN_add(r.bn, a.bn, b.bn) == 0) {
        throw bignum_error("CBigNum::operator+ : BN_add failed");
    }
    return r;
}

inline const CBigNum operator-(const CBigNum& a, const CBigNum& b)
{
    CBigNum r;
    if (BN_sub(r.bn, a.bn, b.bn) == 0) {
        throw bignum_error("CBigNum::operator- : BN_sub failed");
    }
    return r;
}

inline const CBigNum operator-(const CBigNum& a)
{
    CBigNum r(a);
    BN_set_negative(r.bn, static_cast<int>(BN_is_negative(r.bn) == 0));
    return r;
}

inline const CBigNum operator*(const CBigNum& a, const CBigNum& b)
{
    CAutoBN_CTX pctx;
    CBigNum r;
    if (BN_mul(r.bn, a.bn, b.bn, pctx) == 0) {
        throw bignum_error("CBigNum::operator* : BN_mul failed");
    }
    return r;
}

inline const CBigNum operator/(const CBigNum& a, const CBigNum& b)
{
    CAutoBN_CTX pctx;
    CBigNum r;
    if (BN_div(r.bn, nullptr, a.bn, b.bn, pctx) == 0) {
        throw bignum_error("CBigNum::operator/ : BN_div failed");
    }
    return r;
}

inline const CBigNum operator%(const CBigNum& a, const CBigNum& b)
{
    CAutoBN_CTX pctx;
    CBigNum r;
    if (!BN_mod(r.bn, a.bn, b.bn, pctx)) {
        throw bignum_error("CBigNum::operator% : BN_div failed");
    }
    return r;
}

inline const CBigNum operator<<(const CBigNum& a, unsigned int shift)
{
    CBigNum r;
    if (BN_lshift(r.bn, a.bn, shift) == 0) {
        throw bignum_error("CBigNum:operator<< : BN_lshift failed");
    }
    return r;
}

inline const CBigNum operator>>(const CBigNum& a, unsigned int shift)
{
    CBigNum r = a;
    r >>= shift;
    return r;
}

inline const CBigNum operator^(const CBigNum& a, const CBigNum& b)
{
    CBigNum r;
    if (BN_GF2m_add(r.bn, a.bn, b.bn) == 0) {
        throw bignum_error("CBigNum::operator^ : BN_GF2m_add failed");
    }
    return r;
}

inline bool operator==(const CBigNum& a, const CBigNum& b)
{
    return (BN_cmp(a.bn, b.bn) == 0);
}
inline bool operator!=(const CBigNum& a, const CBigNum& b)
{
    return (BN_cmp(a.bn, b.bn) != 0);
}
inline bool operator<=(const CBigNum& a, const CBigNum& b)
{
    return (BN_cmp(a.bn, b.bn) <= 0);
}
inline bool operator>=(const CBigNum& a, const CBigNum& b)
{
    return (BN_cmp(a.bn, b.bn) >= 0);
}
inline bool operator<(const CBigNum& a, const CBigNum& b)
{
    return (BN_cmp(a.bn, b.bn) < 0);
}
inline bool operator>(const CBigNum& a, const CBigNum& b)
{
    return (BN_cmp(a.bn, b.bn) > 0);
}

#endif
