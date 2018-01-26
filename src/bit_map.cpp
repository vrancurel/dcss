
#include "kadsim.h"

void 
BitMap::shuffle()
{
  for (int i = 0;i < n_bits;i++)
    reservoir[i] = i;

  for (int i = n_bits-1;i > 0;--i)
    {
      int j = rand() % i;
      int tmp = reservoir[j];
      reservoir[j] = reservoir[i];
      reservoir[i] = tmp;
    }
}

/** 
 * check that all bits are taken
 * 
 */
bool
BitMap::check()
{
  for (int i = 0;i < n_bits;i++)
    if (get_bit(i))
      return false;

  return true;
}

BitMap::BitMap(int n_bits)
{
  this->n_bits = n_bits;

  b = new char[(n_bits + 7)/8]();

  reservoir = new int[n_bits];
  shuffle();
  pos = 0;
}

BitMap::~BitMap()
{
  delete [] b;
}

void BitMap::set_bit(int i)
{
  b[i / 8] |= 1 << (i & 7);
}

void BitMap::clear_bit(int i)
{
  b[i / 8] &= ~(1 << (i & 7));
}

int BitMap::get_bit(int i)
{
  return b[i / 8] & (1 << (i & 7)) ? 1 : 0;
}

int BitMap::get_rand_bit()
{
  if (pos < n_bits)
    {
      int bit = reservoir[pos];

      //std::cout << "pos " << pos << " bit " << bit << std::endl;

      if (get_bit(bit) != 0)
	{
	  std::cout << "error pos " << pos << " bit " << bit << " is already set" << std::endl;
	  exit(EXIT_FAILURE);
	}
      set_bit(bit);
      pos++;
      return bit;
    }
  else
    {
      std::cout << "error pos=" << pos << " nbits=" << n_bits << std::endl;
      exit(EXIT_FAILURE);
    }
}
