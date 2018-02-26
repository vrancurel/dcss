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
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include "bit_map.h"
#include "exceptions.h"

TEST(BitMapTest, TestCheck) // NOLINT(-*)
{
    const uint32_t nb_bits = 16;
    kad::BitMap bitmap(nb_bits);

    for (uint32_t i = 0; i < nb_bits; ++i) {
        ASSERT_FALSE(bitmap.is_exhausted())
            << "expected " << nb_bits - i << " free bits";
        bitmap.get_rand_uint();
    }
    ASSERT_TRUE(bitmap.is_exhausted()) << "all bits must be used";
    ASSERT_THROW(bitmap.get_rand_uint(), kad::LogicError)
        << "can't call get_rand_uint on an exhausted BitMap";
}

TEST(BitMapTest, TestOutput) // NOLINT(-*)
{
    const uint32_t nb_bits = 16;
    kad::BitMap bitmap(nb_bits);
    std::vector<uint32_t> all_values;
    std::unordered_set<uint32_t> unique_values;
    std::unordered_set<uint32_t> expected;

    for (uint32_t i = 0; i < nb_bits; ++i) {
        expected.insert(i);

        const uint32_t n = bitmap.get_rand_uint();
        all_values.push_back(n);
        unique_values.insert(n);
    }

    EXPECT_EQ(all_values.size(), unique_values.size())
        << "all values must be uniques";
    EXPECT_EQ(expected, unique_values)
        << "all values in [0; " << nb_bits << "[ must be present";
}
