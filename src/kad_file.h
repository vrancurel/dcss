#ifndef __KAD_FILE_H__
#define __KAD_FILE_H__

#include "kad_routable.h"

namespace kad {

class CBigNum;
class Node;

class File : public Routable {
  public:
    File(const CBigNum& file_id, const Node& ref);
    const Node& get_referencer() const;

    ~File() = default;
    File(File const&) = delete;
    File& operator=(File const& x) = delete;
    File(File&&) = delete;
    File& operator=(File&& x) = delete;

  private:
    const Node* const referencer;
};

} // namespace kad

#endif
