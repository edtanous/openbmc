extern "C"
{
#include <libpdbg.h>
}
#include "attributes_info.H"

#include "phalerror/phal_error.hpp"
#include "procedures/phal/common_utils.hpp"

#include <fmt/format.h>
#include <libekb.H>

#include <phosphor-logging/log.hpp>

namespace openpower
{
namespace phal
{

using namespace phosphor::logging;

void phal_init(enum ipl_mode mode)
{
    // TODO: Setting boot error callback should not be in common code
    //       because, we wont get proper reason in PEL for failure.
    //       So, need to make code like caller of this function pass error
    //       handling callback.
    // add callback methods for debug traces and for boot failures
    openpower::pel::addBootErrorCallbacks();

    // PDBG_DTB environment variable set to CEC device tree path
    static constexpr auto PDBG_DTB_PATH =
        "/var/lib/phosphor-software-manager/pnor/rw/DEVTREE";

    if (setenv("PDBG_DTB", PDBG_DTB_PATH, 1))
    {
        log<level::ERR>(
            fmt::format("Failed to set PDBG_DTB: ({})", strerror(errno))
                .c_str());
        throw std::runtime_error("Failed to set PDBG_DTB");
    }

    if (!pdbg_targets_init(NULL))
    {
        log<level::ERR>("pdbg_targets_init failed");
        throw std::runtime_error("pdbg target initialization failed");
    }

    if (libekb_init())
    {
        log<level::ERR>("libekb_init failed");
        throw std::runtime_error("libekb initialization failed");
    }

    if (ipl_init(mode) != 0)
    {
        log<level::ERR>("ipl_init failed");
        throw std::runtime_error("libipl initialization failed");
    }
}

bool isPrimaryProc(struct pdbg_target* procTarget)
{
    ATTR_PROC_MASTER_TYPE_Type type;

    // Get processor type (Primary or Secondary)
    if (DT_GET_PROP(ATTR_PROC_MASTER_TYPE, procTarget, type))
    {
        log<level::ERR>("Attribute [ATTR_PROC_MASTER_TYPE] get failed");
        throw std::runtime_error(
            "Attribute [ATTR_PROC_MASTER_TYPE] get failed");
    }

    /* Attribute value 0 corresponds to primary processor */
    if (type == ENUM_ATTR_PROC_MASTER_TYPE_ACTING_MASTER)
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint32_t getCFAM(struct pdbg_target* procTarget, const uint32_t reg,
                 uint32_t& val)
{
    auto procIdx = pdbg_target_index(procTarget);
    char path[16];
    sprintf(path, "/proc%d/pib", procIdx);

    pdbg_target* pibTarget = pdbg_target_from_path(nullptr, path);
    if (nullptr == pibTarget)
    {
        log<level::ERR>("pib path of target not found",
                        entry("TARGET_PATH=%s", path));
        return -1;
    }

    // probe PIB and ensure it's enabled
    if (PDBG_TARGET_ENABLED != pdbg_target_probe(pibTarget))
    {
        log<level::ERR>("probe on pib target failed");
        return -1;
    }

    // now build FSI path and read the input reg
    sprintf(path, "/proc%d/fsi", procIdx);
    pdbg_target* fsiTarget = pdbg_target_from_path(nullptr, path);
    if (nullptr == fsiTarget)
    {
        log<level::ERR>("fsi path or target not found");
        return -1;
    }

    auto rc = fsi_read(fsiTarget, reg, &val);
    if (rc)
    {
        log<level::ERR>("failed to read input cfam", entry("RC=%u", rc),
                        entry("CFAM=0x%X", reg), entry("TARGET_PATH=%s", path));
        return rc;
    }
    return 0;
}

} // namespace phal
} // namespace openpower
