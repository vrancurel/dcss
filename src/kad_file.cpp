#include "bignum.h"
#include "kad_file.h"

namespace kad {

File::File(const CBigNum& file_id, const Node& ref)
    : Routable(file_id, KAD_ROUTABLE_FILE), referencer(&ref)
{
}

const Node& File::get_referencer() const
{
    return *referencer;
}

} // namespace kad
