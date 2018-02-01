#ifndef __KAD_BITMAP_H__
#define __KAD_BITMAP_H__

#include <vector>

class BitMap {
  public:
    explicit BitMap(int n_bits);
    ~BitMap();

    BitMap(BitMap const&) = delete;
    BitMap& operator=(BitMap const& x) = delete;
    BitMap(BitMap&&) = delete;
    BitMap& operator=(BitMap&& x) = delete;

    int get_bit(int i);
    int
    get_rand_bit(); /* Get a random bit that has never been generated before. */
    bool check();

  private:
    int n_bits;
    char* b;
    std::vector<int> reservoir;
    int pos;

    void set_bit(int i);
    void clear_bit(int i);
};

#endif
