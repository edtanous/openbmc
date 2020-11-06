
#include <libpdbg.h>

#include <util/pdbg.hpp>

namespace util
{

namespace pdbg
{

//------------------------------------------------------------------------------

const char* getPath(const libhei::Chip& i_chip)
{
    return pdbg_target_path((pdbg_target*)i_chip.getChip());
}

//------------------------------------------------------------------------------

} // namespace pdbg

} // namespace util
