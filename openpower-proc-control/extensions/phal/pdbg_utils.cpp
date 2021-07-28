extern "C"
{
#include <libpdbg.h>
}

#include "extensions/phal/pdbg_utils.hpp"
#include "extensions/phal/phal_error.hpp"

#include <fmt/format.h>

#include <phosphor-logging/log.hpp>

namespace openpower
{
namespace phal
{

using namespace phosphor::logging;

pdbg_target* getFsiTarget(struct pdbg_target* procTarget)
{

    struct pdbg_target* fsiTarget = nullptr;
    pdbg_for_each_target("fsi", procTarget, fsiTarget)
    {
        // grab first one we find
        break;
    }
    if (!fsiTarget)
    {
        log<level::ERR>(
            "fsi path of target not found",
            entry("PROC_TARGET_PATH=%s", pdbg_target_path(procTarget)));
        return nullptr;
    }

    return fsiTarget;
}

uint32_t probeTarget(struct pdbg_target* procTarget)
{
    struct pdbg_target* pibTarget = nullptr;
    pdbg_for_each_target("pib", procTarget, pibTarget)
    {
        // grab first one we find
        break;
    }
    if (!pibTarget)
    {
        log<level::ERR>(
            "pib path of target not found",
            entry("PROC_TARGET_PATH=%s", pdbg_target_path(procTarget)));
        return -1;
    }
    // probe PIB and ensure it's enabled
    if (PDBG_TARGET_ENABLED != pdbg_target_probe(pibTarget))
    {
        log<level::ERR>(
            "probe on pib target failed",
            entry("PIB_TARGET_PATH=%s", pdbg_target_path(pibTarget)));
        return -1;
    }
    return 0;
}

uint32_t getCFAM(struct pdbg_target* procTarget, const uint32_t reg,
                 uint32_t& val)
{

    pdbg_target* fsiTarget = getFsiTarget(procTarget);
    if (nullptr == fsiTarget)
    {
        log<level::ERR>("getCFAM: fsi path or target not found");
        return -1;
    }

    auto rc = probeTarget(procTarget);
    if (rc)
    {
        // probe function logged details to journal
        return rc;
    }

    rc = fsi_read(fsiTarget, reg, &val);
    if (rc)
    {
        log<level::ERR>(
            "failed to read input cfam", entry("RC=%u", rc),
            entry("CFAM=0x%X", reg),
            entry("FSI_TARGET_PATH=%s", pdbg_target_path(fsiTarget)));
        return rc;
    }
    return 0;
}

uint32_t putCFAM(struct pdbg_target* procTarget, const uint32_t reg,
                 const uint32_t val)
{
    pdbg_target* fsiTarget = getFsiTarget(procTarget);
    if (nullptr == fsiTarget)
    {
        log<level::ERR>("putCFAM: fsi path or target not found");
        return -1;
    }

    auto rc = probeTarget(procTarget);
    if (rc)
    {
        // probe function logged details to journal
        return rc;
    }

    rc = fsi_write(fsiTarget, reg, val);
    if (rc)
    {
        log<level::ERR>(
            "failed to write input cfam", entry("RC=%u", rc),
            entry("CFAM=0x%X", reg),
            entry("FSI_TARGET_PATH=%s", pdbg_target_path(fsiTarget)));
        return rc;
    }
    return 0;
}

void setDevtreeEnv()
{
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
}

} // namespace phal
} // namespace openpower
