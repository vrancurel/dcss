
#ifndef __BITMAP_H__
#define __BITMAP_H__ 1

#include "kadsim.h"

class BitMap
{
 public:
  BitMap(int n_bits);
  ~BitMap();

  int get_bit(int i);
  int get_rand_bit(); /* get a random bit that has never been generated before */
  bool check();

 private:
  //DISALLOW_COPY_AND_ASSIGN(BitMap);

  int n_bits;
  char *b;
  int *reservoir;
  int pos;

  void set_bit(int i);
  void clear_bit(int i);

  void shuffle();
};

#endif
