// This file is just here to test the CMake setup
// Should be replaced by real tests ASAP!!!

#include <gtest/gtest.h>

static int Factorial(int n)
{
    int result = 1;
    for (int i = 1; i <= n; i++) {
        result *= i;
    }

    return result;
}

// NOLINTNEXTLINE(modernize-use-equals-*)
TEST(FactorialTest, Negative)
{
    EXPECT_EQ(1, Factorial(-5));
    EXPECT_EQ(1, Factorial(-1));
    EXPECT_GT(Factorial(-10), 0);
}

// NOLINTNEXTLINE(modernize-use-equals-*)
TEST(FactorialTest, Zero)
{
    EXPECT_EQ(1, Factorial(0));
}

// NOLINTNEXTLINE(modernize-use-equals-*)
TEST(FactorialTest, Positive)
{
    EXPECT_EQ(1, Factorial(1));
    EXPECT_EQ(2, Factorial(2));
    EXPECT_EQ(6, Factorial(3));
}
