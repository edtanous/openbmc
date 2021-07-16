extern "C"
{
#include "libpdbg.h"
}

#include "extensions/phal/common_utils.hpp"
#include "extensions/phal/create_pel.hpp"
#include "p10_cfam.hpp"
#include "registration.hpp"

#include <phosphor-logging/log.hpp>

#include <cstdio>
#include <fstream>
#include <memory>

namespace openpower
{
namespace phal
{

using namespace openpower::cfam::p10;
using namespace phosphor::logging;

/**
 * This is the backup plan to ensuring the host is not running before the
 * BMC issues a power off to the system. Prior to this procedure being called,
 * the BMC has tried all other communication mechanisms to talk with the host
 * and they have failed. The design is that the host firmware will write the
 * value 0xA5000001 to Mailbox scratch register 12 when they are up and running
 * to a point where communication to the BMC is no longer required to function.
 * On a power off or shutdown this register is cleared by the host and BMC
 * firmware. If the BMC sees the 0xA5000001 pattern in the scratch register
 * then it assumes the host is running and will leave power on to the system.
 */
void checkHostRunning()
{
    struct pdbg_target* procTarget;

    try
    {
        phal_init();
    }
    catch (std::exception& ex)
    {
        // This should "never" happen so just throw the exception and let
        // our systemd error handling process this
        log<level::ERR>("Exception raised during init PHAL",
                        entry("EXCEPTION=%s", ex.what()));
        throw std::runtime_error("PHAL initialization failed");
    }

    pdbg_for_each_class_target("proc", procTarget)
    {
        // Only check the primary proc
        if (!isPrimaryProc(procTarget))
        {
            continue;
        }

        uint32_t val = 0;
        constexpr uint32_t HOST_RUNNING_INDICATION = 0xA5000001;
        auto rc = getCFAM(procTarget, P10_SCRATCH_REG_12, val);
        if ((rc == 0) && (val != HOST_RUNNING_INDICATION))
        {
            log<level::INFO>("CFAM read indicates host is not running",
                             entry("CFAM=0x%X", val));
            return;
        }

        if (rc != 0)
        {
            // On error, we have to assume host is up so just fall through
            // to code below
            log<level::ERR>("CFAM read error, assume host is running");
        }
        else if (val == HOST_RUNNING_INDICATION)
        {
            // This is not good. Normal communication path to host did not work
            // but CFAM indicates host is running.
            log<level::ERR>("CFAM read indicates host is running");
        }

        // Create an error so user knows system is in a bad state
        openpower::pel::createPEL("org.open_power.PHAL.Error.HostRunning");

        // Create file for host instance and create in filesystem to
        // indicate to services that host is running.
        // This file is cleared by the phosphor-state-manager once the host
        // start target completes.
        constexpr auto HOST_RUNNING_FILE = "/run/openbmc/host@%d-on";
        auto size = std::snprintf(nullptr, 0, HOST_RUNNING_FILE, 0);
        size++; // null
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, HOST_RUNNING_FILE, 0);
        std::ofstream outfile(buf.get());
        outfile.close();
        return;
    }

    // We should "never" make it here. If we did it implies no primary processor
    // was found. Once again, rely on systemd recovery if this happens
    log<level::ERR>("No primary processor found in checkHostRunning");
    throw std::runtime_error("No primary processor found in checkHostRunning");
}

/**
 * The BMC is to make a best effort to clear the CFAM register used by PHYP
 * to indicate it is running when the host is stopped. This procedure will do
 * that.
 */
void clearHostRunning()
{
    struct pdbg_target* procTarget;
    log<level::INFO>("Entering clearHostRunning");

    try
    {
        phal_init();
    }
    catch (std::exception& ex)
    {
        // This should "never" happen so just throw the exception and let
        // our systemd error handling process this
        log<level::ERR>("Exception raised during init PHAL",
                        entry("EXCEPTION=%s", ex.what()));
        throw std::runtime_error("PHAL initialization failed");
    }

    pdbg_for_each_class_target("proc", procTarget)
    {
        // Only check the primary proc
        if (!isPrimaryProc(procTarget))
        {
            continue;
        }

        constexpr uint32_t HOST_NOT_RUNNING_INDICATION = 0;
        auto rc = putCFAM(procTarget, P10_SCRATCH_REG_12,
                          HOST_NOT_RUNNING_INDICATION);
        if (rc != 0)
        {
            log<level::ERR>("CFAM write to clear host running status failed");
        }

        // It's best effort, so just return either way
        return;
    }
    log<level::ERR>("No primary processor found in clearHostRunning");
}

REGISTER_PROCEDURE("checkHostRunning", checkHostRunning)
REGISTER_PROCEDURE("clearHostRunning", clearHostRunning)

} // namespace phal
} // namespace openpower
