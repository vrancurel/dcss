#ifndef __KAD_ROUTABLE_H__
#define __KAD_ROUTABLE_H__

#include <string>

#include "bignum.h"

namespace kad {

enum RoutableType {
    KAD_ROUTABLE_NODE,
    KAD_ROUTABLE_FILE,
};

class Routable {
  public:
    Routable(const CBigNum& entity_id, enum RoutableType entity_type);

    CBigNum get_id() const;
    bool is_remote();
    RoutableType get_type();
    CBigNum distance_to(const Routable& other) const;
    bool operator()(const Routable* first, const Routable* second) const;

  protected:
    CBigNum id;
    RoutableType type;
    std::string addr; // Remote peer IP address, or "" if local.
};

} // namespace kad

#endif
