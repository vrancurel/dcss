#include "kad_routable.h"

KadRoutable::KadRoutable(const CBigNum& id, enum KadRoutableType type)
{
    this->id = id;
    this->type = type;
}

CBigNum KadRoutable::get_id() const
{
    return id;
}

CBigNum KadRoutable::distance_to(const KadRoutable& other) const
{
    return id ^ other.get_id();
}

/** Sort from smallest distance to largest.
 *
 * @return true if first is smaller than second
 */
bool KadRoutable::
operator()(const KadRoutable* first, const KadRoutable* second) const
{
    CBigNum d1 = first->distance_to(*this);
    CBigNum d2 = second->distance_to(*this);
    return d1 < d2;
}
