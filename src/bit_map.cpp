#include <algorithm>
#include <iostream>

#include "bit_map.h"
#include "utils.h"

bool BitMap::is_exhausted() const
{
    for (uint32_t i = 0; i < n_bits; i++) {
        if (get_bit(i) == 0) {
            return false;
        }
    }

    return true;
}

BitMap::BitMap(uint32_t n_bits)
{
    this->n_bits = n_bits;

    b = new char[(n_bits + 7) / 8]();

    reservoir.reserve(n_bits);
    for (uint32_t n = 0; n < n_bits; ++n) {
        reservoir.push_back(n);
    }
    shuffle(reservoir.begin(), reservoir.end(), prng());
    pos = 0;
}

BitMap::~BitMap()
{
    delete[] b;
}

void BitMap::set_bit(uint32_t i)
{
    b[i / 8] |= 1 << (i & 7);
}

void BitMap::clear_bit(uint32_t i)
{
    b[i / 8] &= ~(1 << (i & 7));
}

uint32_t BitMap::get_bit(uint32_t i) const
{
    return (b[i / 8] & (1 << (i & 7))) != 0 ? 1 : 0;
}

uint32_t BitMap::get_rand_bit()
{
    if (pos < n_bits) {
        uint32_t bit = reservoir[pos];

        if (get_bit(bit) != 0) {
            std::cout << "error pos " << pos << " bit " << bit
                      << " is already set" << std::endl;
            exit(EXIT_FAILURE);
        }
        set_bit(bit);
        pos++;
        return bit;
    }
    std::cout << "error pos=" << pos << " nbits=" << n_bits << std::endl;
    exit(EXIT_FAILURE);
}
