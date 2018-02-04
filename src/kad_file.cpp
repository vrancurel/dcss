#include "bignum.h"
#include "kad_file.h"

KadFile::KadFile(const CBigNum& file_id, KadNode* ref)
    : KadRoutable(file_id, KAD_ROUTABLE_FILE)
{
    this->referencer = ref;
}

KadNode* KadFile::get_referencer()
{
    return referencer;
}
