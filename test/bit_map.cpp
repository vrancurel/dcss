#include <cstdint>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include "bit_map.h"

// NOLINTNEXTLINE(modernize-use-equals-*)
TEST(BitMapTest, TestCheck)
{
    const uint32_t nb_bits = 16;
    BitMap bitmap(nb_bits);

    for (uint32_t i = 0; i < nb_bits; ++i) {
        ASSERT_FALSE(bitmap.is_exhausted())
            << "expected " << nb_bits - i << " free bits";
        bitmap.get_rand_uint();
    }
    ASSERT_TRUE(bitmap.is_exhausted()) << "all bits must be used";
    ASSERT_THROW(bitmap.get_rand_uint(), std::logic_error)
        << "can't call get_rand_uint on an exhausted BitMap";
}

// NOLINTNEXTLINE(modernize-use-equals-*)
TEST(BitMapTest, TestOutput)
{
    const uint32_t nb_bits = 16;
    BitMap bitmap(nb_bits);
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
