#include <algorithm>

#include "bit_map.h"
#include "exceptions.h"
#include "utils.h"

namespace kad {

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
        throw LogicError("entropy exhausted");
    }
    return pool[pos++];
}

} // namespace kad
