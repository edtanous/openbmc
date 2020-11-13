#include <util/pdbg.hpp>

namespace util
{

namespace pdbg
{

//------------------------------------------------------------------------------

pdbg_target* getTrgt(const libhei::Chip& i_chip)
{
    return (pdbg_target*)i_chip.getChip();
}

//------------------------------------------------------------------------------

const char* getPath(pdbg_target* i_trgt)
{
    return pdbg_target_path(i_trgt);
}

const char* getPath(const libhei::Chip& i_chip)
{
    return getPath(getTrgt(i_chip));
}

//------------------------------------------------------------------------------

uint32_t getChipPos(pdbg_target* i_trgt)
{
    uint32_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_FAPI_POS", 4, 1, &attr);
    return attr;
}

uint32_t getChipPos(const libhei::Chip& i_chip)
{
    return getChipPos(getTrgt(i_chip));
}

//------------------------------------------------------------------------------

uint8_t getTrgtType(pdbg_target* i_trgt)
{
    uint8_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_TYPE", 1, 1, &attr);
    return attr;
}

uint8_t getTrgtType(const libhei::Chip& i_chip)
{
    return getTrgtType(getTrgt(i_chip));
}

//------------------------------------------------------------------------------

} // namespace pdbg

} // namespace util
