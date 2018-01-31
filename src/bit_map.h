#ifndef __KAD_BITMAP_H__
#define __KAD_BITMAP_H__

#include <cstdint>
#include <vector>

class BitMap {
  public:
    explicit BitMap(uint32_t n_bits);
    ~BitMap();

    BitMap(BitMap const&) = delete;
    BitMap& operator=(BitMap const& x) = delete;
    BitMap(BitMap&&) = delete;
    BitMap& operator=(BitMap&& x) = delete;

    uint32_t get_bit(uint32_t i) const;
    /** Get a random bit that has never been generated before. */
    uint32_t get_rand_bit();
    /** Check that all bits are taken. */
    bool is_exhausted() const;

  private:
    uint32_t n_bits;
    char* b;
    std::vector<uint32_t> reservoir;
    uint32_t pos;

    void set_bit(uint32_t i);
    void clear_bit(uint32_t i);
};

#endif
