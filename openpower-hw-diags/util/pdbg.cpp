#include <assert.h>
#include <config.h>

#include <hei_main.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#ifdef CONFIG_PHAL_API
#include <attributes_info.H>
#endif

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

pdbg_target* getTrgt(const std::string& i_path)
{
    return pdbg_target_from_path(nullptr, i_path.c_str());
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
    assert(TYPE_PROC == getTrgtType(i_procTrgt));

    // Get the pib path.
    char path[16];
    sprintf(path, "/proc%d/pib", pdbg_target_index(i_procTrgt));

    // Return the pib target.
    pdbg_target* pibTrgt = pdbg_target_from_path(nullptr, path);
    assert(nullptr != pibTrgt);

    return pibTrgt;
}

//------------------------------------------------------------------------------

pdbg_target* getFsiTrgt(pdbg_target* i_procTrgt)
{
    // The input target must be a processor.
    assert(TYPE_PROC == getTrgtType(i_procTrgt));

    // Get the fsi path.
    char path[16];
    sprintf(path, "/proc%d/fsi", pdbg_target_index(i_procTrgt));

    // Return the fsi target.
    pdbg_target* fsiTrgt = pdbg_target_from_path(nullptr, path);
    assert(nullptr != fsiTrgt);

    return fsiTrgt;
}

//------------------------------------------------------------------------------

int getScom(pdbg_target* i_trgt, uint64_t i_addr, uint64_t& o_val)
{
    // Only processor targets are supported.
    // TODO: Will need to add OCMB support later.
    assert(TYPE_PROC == getTrgtType(i_trgt));

    auto pibTrgt = util::pdbg::getPibTrgt(i_trgt);

    int rc = pib_read(pibTrgt, i_addr, &o_val);

    if (0 != rc)
    {
        trace::err("pib_read failure: target=%s addr=0x%0" PRIx64,
                   util::pdbg::getPath(pibTrgt), i_addr);
    }

    return rc;
}

//------------------------------------------------------------------------------

int getCfam(pdbg_target* i_trgt, uint32_t i_addr, uint32_t& o_val)
{
    // Only processor targets are supported.
    assert(TYPE_PROC == getTrgtType(i_trgt));

    auto fsiTrgt = util::pdbg::getFsiTrgt(i_trgt);

    int rc = fsi_read(fsiTrgt, i_addr, &o_val);

    if (0 != rc)
    {
        trace::err("fsi_read failure: target=%s addr=0x%08x",
                   util::pdbg::getPath(fsiTrgt), i_addr);
    }

    return rc;
}

//------------------------------------------------------------------------------

// IMPORTANT:
// The ATTR_CHIP_ID attribute will be synced from Hostboot to the BMC at some
// point during the IPL. It is possible that this information is needed before
// the sync occurs, in which case the value will return 0.
uint32_t __getChipId(pdbg_target* i_trgt)
{
    uint32_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_CHIP_ID", 4, 1, &attr);
    return attr;
}

// IMPORTANT:
// The ATTR_EC attribute will be synced from Hostboot to the BMC at some point
// during the IPL. It is possible that this information is needed before the
// sync occurs, in which case the value will return 0.
uint8_t __getChipEc(pdbg_target* i_trgt)
{
    uint8_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_EC", 1, 1, &attr);
    return attr;
}

uint32_t __getChipIdEc(pdbg_target* i_trgt)
{
    auto chipId = __getChipId(i_trgt);
    auto chipEc = __getChipEc(i_trgt);

    if (((0 == chipId) || (0 == chipEc)) && (TYPE_PROC == getTrgtType(i_trgt)))
    {
        // There is a special case where the model/level attributes have not
        // been initialized in the devtree. This is possible on the epoch IPL
        // where an attention occurs before Hostboot is able to update the
        // devtree information on the BMC. It may is still possible to get this
        // information from chips with CFAM access (i.e. a processor) via the
        // CFAM chip ID register.

        uint32_t val = 0;
        if (0 == getCfam(i_trgt, 0x100a, val))
        {
            chipId = ((val & 0x0F0FF000) >> 12);
            chipEc = ((val & 0xF0000000) >> 24) | ((val & 0x00F00000) >> 20);
        }
    }

    return ((chipId & 0xffff) << 16) | (chipEc & 0xff);
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
        // This is a special case. See the details in __getChipIdEC(). There is
        // nothing more we can do with this chip since we don't know what it is.
        // So ignore the chip for now.
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

pdbg_target* getPrimaryProcessor()
{
    // TODO: For at least P10, the primary processor (the one connected directly
    //       to the BMC), will always be PROC 0. We will need to update this
    //       later if we ever support an alternate primary processor.
    return getTrgt("/proc0");
}

//------------------------------------------------------------------------------

bool queryHardwareAnalysisSupported()
{
    // Hardware analysis is only supported on P10 systems and up.
    return (PDBG_PROC_P9 < pdbg_get_proc());
}

//------------------------------------------------------------------------------

std::string getLocationCode(pdbg_target* trgt)
{
    if (nullptr == trgt)
    {
        // Either the path is wrong or the attribute doesn't exist.
        return std::string{};
    }

#ifdef CONFIG_PHAL_API

    ATTR_LOCATION_CODE_Type val;
    if (DT_GET_PROP(ATTR_LOCATION_CODE, trgt, val))
    {
        // Get the immediate parent in the devtree path and try again.
        return getLocationCode(pdbg_target_parent(nullptr, trgt));
    }

    // Attribute found.
    return std::string{val};

#else

    return std::string{getPath(trgt)};

#endif
}

//------------------------------------------------------------------------------

std::string getPhysDevPath(pdbg_target* trgt)
{
    if (nullptr == trgt)
    {
        // Either the path is wrong or the attribute doesn't exist.
        return std::string{};
    }

#ifdef CONFIG_PHAL_API

    ATTR_PHYS_DEV_PATH_Type val;
    if (DT_GET_PROP(ATTR_PHYS_DEV_PATH, trgt, val))
    {
        // Get the immediate parent in the devtree path and try again.
        return getPhysDevPath(pdbg_target_parent(nullptr, trgt));
    }

    // Attribute found.
    return std::string{val};

#else

    return std::string{getPath(trgt)};

#endif
}

//------------------------------------------------------------------------------

std::vector<uint8_t> getPhysBinPath(pdbg_target* target)
{
    std::vector<uint8_t> binPath;

    if (nullptr != target)
    {
#ifdef CONFIG_PHAL_API

        ATTR_PHYS_BIN_PATH_Type value;
        if (DT_GET_PROP(ATTR_PHYS_BIN_PATH, target, value))
        {
            // The attrirbute for this target does not exist. Get the immediate
            // parent in the devtree path and try again. Note that if there is
            // no parent target, nullptr will be returned and that will be
            // checked above.
            return getPhysBinPath(pdbg_target_parent(nullptr, target));
        }

        // Attribute was found. Copy the attribute array to the returned
        // vector. Note that the reason we return the vector instead of just
        // returning the array is because the array type and details only
        // exists in this specific configuration.
        binPath.insert(binPath.end(), value, value + sizeof(value));

#endif
    }

    return binPath;
}

//------------------------------------------------------------------------------

} // namespace pdbg

} // namespace util
