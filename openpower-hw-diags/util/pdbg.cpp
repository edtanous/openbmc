//------------------------------------------------------------------------------
// IMPORTANT:
// This file will be built in CI test and should work out-of-the-box in CI test
// with use of the fake device tree. Any functions that require addition support
// to simulate in CI test should be put in `pdbg_no_sim.cpp`.
//------------------------------------------------------------------------------

#include <assert.h>
#include <config.h>

#include <hei_main.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#ifdef CONFIG_PHAL_API
#include <attributes_info.H>
#endif

using namespace analyzer;

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

uint8_t getUnitPos(pdbg_target* i_trgt)
{
    uint8_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_CHIP_UNIT_POS", 1, 1, &attr);
    return attr;
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

pdbg_target* getParentChip(pdbg_target* i_unitTarget)
{
    assert(nullptr != i_unitTarget);

    // Check if the given target is already a chip.
    auto targetType = getTrgtType(i_unitTarget);
    if (TYPE_PROC == targetType || TYPE_OCMB == targetType)
    {
        return i_unitTarget; // simply return the given target
    }

    // Check if this unit is on an OCMB.
    pdbg_target* parentChip = pdbg_target_parent("ocmb", i_unitTarget);

    // If not on the OCMB, check if this unit is on a PROC.
    if (nullptr == parentChip)
    {
        parentChip = pdbg_target_parent("proc", i_unitTarget);
    }

    // There should always be a parent chip. Throw an error if not found.
    if (nullptr == parentChip)
    {
        throw std::logic_error("No parent chip found: i_unitTarget=" +
                               std::string{getPath(i_unitTarget)});
    }

    return parentChip;
}

//------------------------------------------------------------------------------

pdbg_target* getChipUnit(pdbg_target* i_parentChip, TargetType_t i_unitType,
                         uint8_t i_unitPos)
{
    assert(nullptr != i_parentChip);

    auto parentType = getTrgtType(i_parentChip);

    std::string devTreeType{};

    if (TYPE_PROC == parentType)
    {
        // clang-format off
        static const std::map<TargetType_t, std::string> m =
        {
            {TYPE_MC,     "mc"      },
            {TYPE_MCC,    "mcc"     },
            {TYPE_OMI,    "omi"     },
            {TYPE_OMIC,   "omic"    },
            {TYPE_PAUC,   "pauc"    },
            {TYPE_PAU,    "pau"     },
            {TYPE_NMMU,   "nmmu"    },
            {TYPE_IOHS,   "iohs"    },
            {TYPE_IOLINK, "smpgroup"},
            {TYPE_EQ,     "eq"      },
            {TYPE_CORE,   "core"    },
            {TYPE_PEC,    "pec"     },
            {TYPE_PHB,    "phb"     },
            {TYPE_NX,     "nx"      },
        };
        // clang-format on

        devTreeType = m.at(i_unitType);
    }
    else if (TYPE_OCMB == parentType)
    {
        // clang-format off
        static const std::map<TargetType_t, std::string> m =
        {
            {TYPE_MEM_PORT, "mem_port"},
        };
        // clang-format on

        devTreeType = m.at(i_unitType);
    }
    else
    {
        throw std::logic_error("Unexpected parent chip: " +
                               std::string{getPath(i_parentChip)});
    }

    // Iterate all children of the parent and match the unit position.
    pdbg_target* unitTarget = nullptr;
    pdbg_for_each_target(devTreeType.c_str(), i_parentChip, unitTarget)
    {
        if (nullptr != unitTarget && i_unitPos == getUnitPos(unitTarget))
        {
            break; // found it
        }
    }

    // Print a warning if the target unit is not found, but don't throw an
    // error.  Instead let the calling code deal with the it.
    if (nullptr == unitTarget)
    {
        trace::err("No unit target found: i_parentChip=%s i_unitType=0x%02x "
                   "i_unitPos=%u",
                   getPath(i_parentChip), i_unitType, i_unitPos);
    }

    return unitTarget;
}

//------------------------------------------------------------------------------

pdbg_target* getConnectedTarget(pdbg_target* i_rxTarget,
                                const callout::BusType& i_busType)
{
    assert(nullptr != i_rxTarget);

    pdbg_target* txTarget = nullptr;

    auto rxType        = util::pdbg::getTrgtType(i_rxTarget);
    std::string rxPath = util::pdbg::getPath(i_rxTarget);

    if (callout::BusType::SMP_BUS == i_busType &&
        util::pdbg::TYPE_IOLINK == rxType)
    {
        // TODO: Will need to reference some sort of data that can tell us how
        //       the processors are connected in the system. For now, return the
        //       RX target to avoid returning a nullptr.
        trace::inf("No support to get peer target on SMP bus");
        txTarget = i_rxTarget;
    }
    else if (callout::BusType::SMP_BUS == i_busType &&
             util::pdbg::TYPE_IOHS == rxType)
    {
        // TODO: Will need to reference some sort of data that can tell us how
        //       the processors are connected in the system. For now, return the
        //       RX target to avoid returning a nullptr.
        trace::inf("No support to get peer target on SMP bus");
        txTarget = i_rxTarget;
    }
    else if (callout::BusType::OMI_BUS == i_busType &&
             util::pdbg::TYPE_OMI == rxType)
    {
        // This is a bit clunky. The pdbg APIs only give us the ability to
        // iterate over the children instead of just returning a list. So we'll
        // push all the children to a list and go from there.
        std::vector<pdbg_target*> childList;

        pdbg_target* childTarget = nullptr;
        pdbg_for_each_target("ocmb", i_rxTarget, childTarget)
        {
            if (nullptr != childTarget)
            {
                childList.push_back(childTarget);
            }
        }

        // We know there should only be one OCMB per OMI.
        if (1 != childList.size())
        {
            throw std::logic_error("Invalid child list size for " + rxPath);
        }

        // Get the connected target.
        txTarget = childList.front();
    }
    else if (callout::BusType::OMI_BUS == i_busType &&
             util::pdbg::TYPE_OCMB == rxType)
    {
        txTarget = pdbg_target_parent("omi", i_rxTarget);
        if (nullptr == txTarget)
        {
            throw std::logic_error("No parent OMI found for " + rxPath);
        }
    }
    else
    {
        // This would be a code bug.
        throw std::logic_error("Unsupported config: i_rxTarget=" + rxPath +
                               " i_busType=" + i_busType.getString());
    }

    assert(nullptr != txTarget); // just in case we missed something above

    return txTarget;
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

void getActiveProcessorChips(std::vector<pdbg_target*>& o_chips)
{
    o_chips.clear();

    pdbg_target* procTrgt;
    pdbg_for_each_class_target("proc", procTrgt)
    {
        // We cannot use the proc target to determine if the chip is active.
        // There is some design limitation in pdbg that requires the proc
        // targets to always be active. Instead, we must get the associated pib
        // target and check if it is active.

        if (PDBG_TARGET_ENABLED != pdbg_target_probe(getPibTrgt(procTrgt)))
            continue;

        o_chips.push_back(procTrgt);
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
