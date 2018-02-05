#ifndef __KAD_UTILS_H__
#define __KAD_UTILS_H__

#include <cstdint>
#include <limits>
#include <random>
#include <string>

#include "exceptions.h"

static inline uint32_t
stou32(std::string const& str, size_t* idx = nullptr, int base = 10)
{
    // NOLINTNEXTLINE(google-runtime-int)
    unsigned long result = std::stoul(str, idx, base);

    if (result > std::numeric_limits<uint32_t>::max()) {
        throw KadDomainError("stou32");
    }
    return static_cast<uint32_t>(result);
}

static inline uint64_t
stou64(std::string const& str, size_t* idx = nullptr, int base = 10)
{
    // NOLINTNEXTLINE(google-runtime-int)
    unsigned long long result = std::stoull(str, idx, base);

    if (result > std::numeric_limits<uint64_t>::max()) {
        throw KadDomainError("stou64");
    }
    return static_cast<uint64_t>(result);
}

// Return a reference to the global PRNG.
static inline std::mt19937& prng()
{
    static std::mt19937 PRNG;

    return PRNG;
}

#endif
