#include <algorithm>
#include <stdexcept>

#include "bit_map.h"
#include "utils.h"

bool BitMap::is_exhausted() const
{
    return pos == pool.size();
}

BitMap::BitMap(uint32_t n_bits) : pos(0)
{
    pool.reserve(n_bits);
    for (uint32_t n = 0; n < n_bits; ++n) {
        pool.push_back(n);
    }
    shuffle(pool.begin(), pool.end(), prng());
}

uint32_t BitMap::get_rand_uint()
{
    if (is_exhausted()) {
        // std::logic_error IS nothrow copy constructible.
        throw std::logic_error("entropy exhausted"); // NOLINT(cert-err60-cpp)
    }
    return pool[pos++];
}
