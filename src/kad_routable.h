#ifndef __KAD_ROUTABLE_H__
#define __KAD_ROUTABLE_H__

#include <string>

#include "bignum.h"

enum KadRoutableType {
    KAD_ROUTABLE_NODE,
    KAD_ROUTABLE_FILE,
};

class KadRoutable {
  public:
    KadRoutable(const CBigNum& id, enum KadRoutableType);

    CBigNum get_id() const;
    bool is_remote();
    KadRoutableType get_type();
    CBigNum distance_to(const KadRoutable& other) const;
    bool operator()(const KadRoutable* first, const KadRoutable* second) const;

  protected:
    CBigNum id;
    KadRoutableType type;
    std::string addr; // Remote peer IP address, or "" if local.
};

#endif
