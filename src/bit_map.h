#ifndef __KAD_BITMAP_H__
#define __KAD_BITMAP_H__

#include <cstdint>
#include <vector>

namespace kad {

// If the performance/space usage ever becomes an issue, we may want to consider
// an approach based on the quadratic residues (O(1) in time and space).
class BitMap {
  public:
    explicit BitMap(uint32_t n_bits);

    /** Get a random value that has never been generated before. */
    uint32_t get_rand_uint();
    /** Check if the entropy is exhausted. */
    bool is_exhausted() const;

  private:
    std::vector<uint32_t> pool;
    uint32_t pos;
};

} // namespace kad

#endif
