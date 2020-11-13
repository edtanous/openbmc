#include <assert.h>

#include <util/pdbg.hpp>
#include <util/trace.hpp>

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

pdbg_target* getPibTrgt(pdbg_target* i_procTrgt)
{
    // The input target must be a processor.
    assert(0x05 == getTrgtType(i_procTrgt));

    // Get the pib path.
    char path[16];
    sprintf(path, "/proc%d/pib", pdbg_target_index(i_procTrgt));

    // Return the pib target.
    pdbg_target* pibTrgt = pdbg_target_from_path(nullptr, path);
    assert(nullptr != pibTrgt);

    return pibTrgt;
}

//------------------------------------------------------------------------------

uint32_t __getChipId(pdbg_target* i_trgt)
{
    uint32_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_CHIP_ID", 4, 1, &attr);
    return attr;
}

uint8_t __getChipEc(pdbg_target* i_trgt)
{
    uint8_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_EC", 1, 1, &attr);
    return attr;
}

uint32_t __getChipIdEc(pdbg_target* i_trgt)
{
    return ((__getChipId(i_trgt) & 0xffff) << 16) | __getChipEc(i_trgt);
}

void __addChip(std::vector<libhei::Chip>& o_chips, pdbg_target* i_trgt,
               libhei::ChipType_t i_type)
{
    // Trace each chip for debug. It is important to show the type just in case
    // the model/EC does not exist. See note below.
    trace::inf("Chip found: type=0x%08" PRIx32 " chip=%s", i_type,
               getPath(i_trgt));

    if (0 == i_type)
    {
        // There is a special case where the model/level attributes have not
        // been initialized in the devtree. This is possible on the epoch IPL
        // where an attention occurs before Hostboot is able to update the
        // devtree information on the BMC. For now, just ignore the chip.
    }
    else
    {
        o_chips.emplace_back(i_trgt, i_type);
    }
}

void getActiveChips(std::vector<libhei::Chip>& o_chips)
{
    o_chips.clear();

    // Iterate each processor.
    pdbg_target* procTrgt;
    pdbg_for_each_class_target("proc", procTrgt)
    {
        // We cannot use the proc target to determine if the chip is active.
        // There is some design limitation in pdbg that requires the proc
        // targets to always be active. Instead, we must get the associated pib
        // target and check if it is active.

        // Active processors only.
        if (PDBG_TARGET_ENABLED != pdbg_target_probe(getPibTrgt(procTrgt)))
            continue;

        // Add the processor to the list.
        __addChip(o_chips, procTrgt, __getChipIdEc(procTrgt));

        // Iterate the connected OCMBs, if they exist.
        pdbg_target* ocmbTrgt;
        pdbg_for_each_target("ocmb", procTrgt, ocmbTrgt)
        {
            // Active OCMBs only.
            if (PDBG_TARGET_ENABLED != pdbg_target_probe(ocmbTrgt))
                continue;

            // Add the OCMB to the list.
            __addChip(o_chips, ocmbTrgt, __getChipIdEc(ocmbTrgt));
        }
    }
}

//------------------------------------------------------------------------------

} // namespace pdbg

} // namespace util
