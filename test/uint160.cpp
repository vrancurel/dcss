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
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "exceptions.h"
#include "uint160.h"

template <typename T>
std::string int_to_hex(T n)
{
    std::stringstream stream;

    stream << std::setfill('0') << std::setw(40) << std::hex << n;
    return stream.str();
}

TEST(UInt160Test, TestInitDefault) // NOLINT
{
    const kad::UInt160 n{};
    const kad::UInt160 zero(0u);

    // NOLINTNEXTLINE(hicpp-vararg)
    EXPECT_EQ(n, zero) << "default constructor return the value 0";
}

TEST(UInt160Test, TestInitFromInt) // NOLINT
{
    const uint64_t values[] = {0, 1, 255, 65000, 4000000, 10000000000};

    for (auto val : values) {
        const std::string result(kad::UInt160(val).to_string());
        const std::string expected(int_to_hex(val));

        // NOLINTNEXTLINE(hicpp-vararg)
        EXPECT_EQ(result, expected) << "testing init from " << val;
    }
}

TEST(UInt160Test, TestInitFromHex) // NOLINT
{
    const std::string hex("c544b5e4a1afcbb5d2de772d7a8df76f32557147");
    kad::UInt160 n(hex);

    // NOLINTNEXTLINE(hicpp-vararg)
    EXPECT_EQ(n.to_string(), hex) << "testing init from " << hex;

    ASSERT_THROW(kad::UInt160("deadbeef"), kad::LogicError)
        << "bad hex string size";

    ASSERT_THROW(
        kad::UInt160("One cannot step twice in the same river."),
        kad::Exception)
        << "bad string (not an hex string)";
}

TEST(UInt160Test, TestBitLength) // NOLINT
{
    const std::pair<kad::UInt160, int> testcases[] = {
        std::make_pair(0u, 0),
        std::make_pair(1u, 1),
        std::make_pair(2u, 2),
        std::make_pair(3u, 2),
        std::make_pair(4u, 3),
        std::make_pair(5u, 3),
        std::make_pair(6u, 3),
        std::make_pair(7u, 3),
        std::make_pair(
            kad::UInt160("8f0b49e7cdc5c120599cfe86886b622b2969e24f"), 160)};

    for (const auto& test : testcases) {
        // NOLINTNEXTLINE(hicpp-vararg)
        EXPECT_EQ(test.first.bit_length(), test.second)
            << "testing bit length of " << test.first;
    }
}

TEST(UInt160Test, TestBoolContext) // NOLINT
{
    const kad::UInt160 zero(0u);
    const kad::UInt160 n(42u);

    ASSERT_FALSE(bool(zero)) << "zero is false";
    ASSERT_TRUE(bool(n)) << "non zero is true";

    ASSERT_TRUE(!zero) << "!zero is true ";
    ASSERT_FALSE(!n) << "!non zero is false";

    ASSERT_FALSE(zero && zero) << "test false && false ";
    ASSERT_FALSE(zero && n) << "test false && true";
    ASSERT_FALSE(n && zero) << "test true && false";
    ASSERT_TRUE(n && n) << "test true && true ";

    ASSERT_FALSE(zero || zero) << "test false || false ";
    ASSERT_TRUE(zero || n) << "test false || true";
    ASSERT_TRUE(n || zero) << "test true || false";
    ASSERT_TRUE(n || n) << "test true || true ";
}

TEST(UInt160Test, TestOrdering) // NOLINT
{
    const kad::UInt160 a("3198e90e9149480c3d76354e1cbbdbff5c80b590");
    const kad::UInt160 b("97a149c721c981fe4e3a7440f6227ac4797bc238");

    ASSERT_TRUE(a == a) << "test ==";
    ASSERT_TRUE(a != b) << "test !=";

    ASSERT_TRUE(a < b) << "test <";
    ASSERT_TRUE(a <= b) << "test <=";

    ASSERT_FALSE(a > b) << "test >";
    ASSERT_FALSE(a >= b) << "test >=";

    ASSERT_TRUE(a <= a) << "test <= on itself";
    ASSERT_TRUE(a >= a) << "test >= on itself";
}

TEST(UInt160Test, TestPositive) // NOLINT
{
    const kad::UInt160 zero(0u);
    const kad::UInt160 one(1u);
    const kad::UInt160 a("b476a316af1a0a53984c3e4324b3383c0640fd38");

    ASSERT_EQ(+zero, 0u) << "test +0"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(+one, 1u) << "test +1";  // NOLINT(hicpp-vararg)
    ASSERT_EQ(+a, a) << "test +a";     // NOLINT(hicpp-vararg)
}

TEST(UInt160Test, TestNegate) // NOLINT
{
    const kad::UInt160 zero(0u);
    const kad::UInt160 one(1u);
    const kad::UInt160 neg_1("ffffffffffffffffffffffffffffffffffffffff");
    const kad::UInt160 a("229bb29286f7424f2f004f415d02efcd8f2b2fec");
    const kad::UInt160 neg_a("dd644d6d7908bdb0d0ffb0bea2fd103270d4d014");

    ASSERT_EQ(-zero, zero) << "test -0"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(-one, neg_1) << "test -1"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(-a, neg_a) << "test -a";   // NOLINT(hicpp-vararg)
}

TEST(UInt160Test, TestAdd) // NOLINT
{
    const kad::UInt160 a("bd6756490ba3b01c0ea5ce627e7fc5b16bbe10e2");
    const kad::UInt160 b("981bbf665b803744f91960058d69e4fb72ac3b18");
    const kad::UInt160 expected("558315af6723e76107bf2e680be9aaacde6a4bfa");

    ASSERT_EQ(a + b, expected) << "test a+b"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(b + a, expected) << "test b+a"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(a + 0u, a) << "test a+0";       // NOLINT(hicpp-vararg)
    ASSERT_EQ(0u + a, a) << "test 0+a";       // NOLINT(hicpp-vararg)
}

TEST(UInt160Test, TestSub) // NOLINT
{
    const kad::UInt160 a("49af33926747cb8765570a7d156b665984c6f03e");
    const kad::UInt160 b("501014d4411ca9e0be1c3db692b82a7bdf42e8c5");
    const kad::UInt160 a_min_b("f99f1ebe262b21a6a73accc682b33bdda5840779");
    const kad::UInt160 b_min_a("0660e141d9d4de5958c533397d4cc4225a7bf887");

    ASSERT_EQ(a - 0u, a) << "test a-0";      // NOLINT(hicpp-vararg)
    ASSERT_EQ(0u - a, -a) << "test 0-a";     // NOLINT(hicpp-vararg)
    ASSERT_EQ(a - a, 0u) << "test a-a";      // NOLINT(hicpp-vararg)
    ASSERT_EQ(a - b, a_min_b) << "test a-b"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(b - a, b_min_a) << "test b-a"; // NOLINT(hicpp-vararg)
}

TEST(UInt160Test, TestInc) // NOLINT
{
    kad::UInt160 n("22887ffeffe10cd8df3526a647193b59ddf1a55a");
    kad::UInt160 n_ref(n);
    const kad::UInt160 n_plus_1("22887ffeffe10cd8df3526a647193b59ddf1a55b");
    const kad::UInt160 n_plus_2("22887ffeffe10cd8df3526a647193b59ddf1a55c");

    ASSERT_EQ(n++, n_ref) << "test n++ (1/2)";  // NOLINT(hicpp-vararg)
    ASSERT_EQ(n, n_plus_1) << "test n++ (2/2)"; // NOLINT(hicpp-vararg)

    ASSERT_EQ(++n, n_plus_2) << "test ++n (1/2)"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(n, n_plus_2) << "test ++n (2/2)";   // NOLINT(hicpp-vararg)
}

TEST(UInt160Test, TestDec) // NOLINT
{
    kad::UInt160 n("7de849d8e2341271dcf1d88ec86a2cfc9e4dfd08");
    kad::UInt160 n_ref(n);
    const kad::UInt160 n_min_1("7de849d8e2341271dcf1d88ec86a2cfc9e4dfd07");
    const kad::UInt160 n_min_2("7de849d8e2341271dcf1d88ec86a2cfc9e4dfd06");

    ASSERT_EQ(n--, n_ref) << "test n-- (1/2)"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(n, n_min_1) << "test n-- (2/2)"; // NOLINT(hicpp-vararg)

    ASSERT_EQ(--n, n_min_2) << "test --n (1/2)"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(n, n_min_2) << "test --n (2/2)";   // NOLINT(hicpp-vararg)
}

TEST(UInt160Test, TestBitwiseLogical) // NOLINT
{
    const kad::UInt160 a("6ebee938eda3024d35d74729d7475a6b826a4a70");
    const kad::UInt160 b("3af7f7bf61bd037eaee1b43bb895b57a21b4deb4");
    const kad::UInt160 a_and_b("2ab6e13861a1024c24c104299005106a00204a30");
    const kad::UInt160 a_or_b("7effffbfedbf037fbff7f73bffd7ff7ba3fedef4");
    const kad::UInt160 a_xor_b("54491e878c1e01339b36f3126fd2ef11a3de94c4");
    const kad::UInt160 not_a("914116c7125cfdb2ca28b8d628b8a5947d95b58f");

    ASSERT_EQ(a & b, a_and_b) << "test &"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(a | b, a_or_b) << "test |";  // NOLINT(hicpp-vararg)
    ASSERT_EQ(a ^ b, a_xor_b) << "test ^"; // NOLINT(hicpp-vararg)
    ASSERT_EQ(~a, not_a) << "test ~";      // NOLINT(hicpp-vararg)
}

TEST(UInt160Test, TestLeftShift) // NOLINT
{
    kad::UInt160 n("7E4A78FD337C596B351BC7645EA266ADCE6EF780");
    const std::pair<unsigned, std::string> testcases[] = {
        std::make_pair(0, "7e4a78fd337c596b351bc7645ea266adce6ef780"),
        std::make_pair(7, "253c7e99be2cb59a8de3b22f513356e7377bc000"),
        std::make_pair(32, "337c596b351bc7645ea266adce6ef78000000000"),
        std::make_pair(49, "b2d66a378ec8bd44cd5b9cddef00000000000000"),
        std::make_pair(64, "351bc7645ea266adce6ef7800000000000000000"),
        std::make_pair(68, "51bc7645ea266adce6ef78000000000000000000"),
        std::make_pair(96, "5ea266adce6ef780000000000000000000000000"),
        std::make_pair(103, "513356e7377bc000000000000000000000000000"),
        std::make_pair(128, "ce6ef78000000000000000000000000000000000"),
        std::make_pair(139, "77bc000000000000000000000000000000000000"),
        std::make_pair(160, "0000000000000000000000000000000000000000"),
        std::make_pair(192, "0000000000000000000000000000000000000000"),
    };

    for (const auto& test : testcases) {
        const unsigned shift = test.first;
        const kad::UInt160 result = n << shift;

        // NOLINTNEXTLINE(hicpp-vararg)
        EXPECT_EQ(result.to_string(), test.second) << "testing: n << " << shift;
    }
}

TEST(UInt160Test, TestRightShift) // NOLINT
{
    kad::UInt160 n("6ADC6A3DB11C764E1BE600992CAFBEB7C293C068");
    const std::pair<unsigned, std::string> testcases[] = {
        std::make_pair(0, "6adc6a3db11c764e1be600992cafbeb7c293c068"),
        std::make_pair(19, "00000d5b8d47b6238ec9c37cc0132595f7d6f852"),
        std::make_pair(32, "000000006adc6a3db11c764e1be600992cafbeb7"),
        std::make_pair(53, "0000000000000356e351ed88e3b270df3004c965"),
        std::make_pair(64, "00000000000000006adc6a3db11c764e1be60099"),
        std::make_pair(70, "000000000000000001ab71a8f6c471d9386f9802"),
        std::make_pair(96, "0000000000000000000000006adc6a3db11c764e"),
        std::make_pair(121, "000000000000000000000000000000356e351ed8"),
        std::make_pair(128, "000000000000000000000000000000006adc6a3d"),
        std::make_pair(147, "0000000000000000000000000000000000000d5b"),
        std::make_pair(160, "0000000000000000000000000000000000000000"),
        std::make_pair(192, "0000000000000000000000000000000000000000"),
    };
    for (const auto& test : testcases) {
        const unsigned shift = test.first;
        const kad::UInt160 result = n >> shift;

        // NOLINTNEXTLINE(hicpp-vararg)
        EXPECT_EQ(result.to_string(), test.second) << "testing: n >> " << shift;
    }
}
