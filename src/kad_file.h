#ifndef __KAD_FILE_H__
#define __KAD_FILE_H__

#include "kad_routable.h"

class CBigNum;
class KadNode;

class KadFile : public KadRoutable {
  public:
    KadFile(const CBigNum& id, KadNode* referencer);
    KadNode* get_referencer();

    ~KadFile() = default;
    KadFile(KadFile const&) = delete;
    KadFile& operator=(KadFile const& x) = delete;
    KadFile(KadFile&&) = delete;
    KadFile& operator=(KadFile&& x) = delete;

  private:
    KadNode* referencer;
};

#endif
