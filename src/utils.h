#ifndef __KAD_UTILS_H__
#define __KAD_UTILS_H__

#include <random>

// Return a reference to the global PRNG.
static inline std::mt19937& prng()
{
    static std::mt19937 PRNG;

    return PRNG;
}

#endif
