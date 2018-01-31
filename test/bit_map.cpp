#include <cstdint>

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
        bitmap.get_rand_bit();
    }
    ASSERT_TRUE(bitmap.is_exhausted()) << "all bits must be used";
    ASSERT_EXIT(bitmap.get_rand_bit(), ::testing::ExitedWithCode(EXIT_FAILURE),
            "") << "can't call get_rand_bit on an exhausted BitMap";
}
