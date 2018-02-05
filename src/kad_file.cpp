#include "bignum.h"
#include "kad_file.h"

namespace kad {

File::File(const CBigNum& file_id, Node* ref)
    : Routable(file_id, KAD_ROUTABLE_FILE)
{
    this->referencer = ref;
}

Node* File::get_referencer()
{
    return referencer;
}

} // namespace kad
