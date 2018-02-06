#include "kad_routable.h"

namespace kad {

Routable::Routable(const CBigNum& entity_id, enum RoutableType entity_type)
{
    this->id = entity_id;
    this->type = entity_type;
}

CBigNum Routable::get_id() const
{
    return id;
}

CBigNum Routable::distance_to(const Routable& other) const
{
    return id ^ other.get_id();
}

/** Sort from smallest distance to largest.
 *
 * @return true if first is smaller than second
 */
bool Routable::operator()(const Routable* first, const Routable* second) const
{
    CBigNum d1 = first->distance_to(*this);
    CBigNum d2 = second->distance_to(*this);
    return d1 < d2;
}

} // namespace kad
