extern "C"
{
#include <libpdbg.h>
}

#include "attributes_info.H"

#include "phalerror/phal_error.hpp"
#include "procedures/phal/common_utils.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <libekb.H>

#include <ext_interface.hpp>
#include <phosphor-logging/log.hpp>
#include <registration.hpp>

namespace openpower
{
namespace phal
{

using namespace phosphor::logging;

/**
 *  @brief  Check if master processor or not
 *
 *  @return True/False
 */
bool isMasterProc(struct pdbg_target* procTarget)
{
    ATTR_PROC_MASTER_TYPE_Type type;

    // Get processor type (Master or Alt-master)
    if (DT_GET_PROP(ATTR_PROC_MASTER_TYPE, procTarget, type))
    {
        log<level::ERR>("Attribute [ATTR_PROC_MASTER_TYPE] get failed");
        throw std::runtime_error(
            "Attribute [ATTR_PROC_MASTER_TYPE] get failed");
    }

    /* Attribute value 0 corresponds to master processor */
    if (type == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 *  @brief  Select BOOT SEEPROM and Measurement SEEPROM(PRIMARY/BACKUP) on POWER
 *          processor position 0/1 depending on boot count before kicking off
 *          the boot.
 *
 *  @return void
 */
void selectBootSeeprom()
{
    struct pdbg_target* procTarget;
    ATTR_BACKUP_SEEPROM_SELECT_Enum bkpSeePromSelect;
    ATTR_BACKUP_MEASUREMENT_SEEPROM_SELECT_Enum bkpMeaSeePromSelect;

    pdbg_for_each_class_target("proc", procTarget)
    {
        if (!isMasterProc(procTarget))
        {
            continue;
        }

        // Choose seeprom side to boot from based on boot count
        if (getBootCount() > 0)
        {
            log<level::INFO>("Setting SBE seeprom side to 0",
                             entry("SBE_SIDE_SELECT=%d",
                                   ENUM_ATTR_BACKUP_SEEPROM_SELECT_PRIMARY));

            bkpSeePromSelect = ENUM_ATTR_BACKUP_SEEPROM_SELECT_PRIMARY;
            bkpMeaSeePromSelect =
                ENUM_ATTR_BACKUP_MEASUREMENT_SEEPROM_SELECT_PRIMARY;
        }
        else
        {
            log<level::INFO>("Setting SBE seeprom side to 1",
                             entry("SBE_SIDE_SELECT=%d",
                                   ENUM_ATTR_BACKUP_SEEPROM_SELECT_SECONDARY));
            bkpSeePromSelect = ENUM_ATTR_BACKUP_SEEPROM_SELECT_SECONDARY;
            bkpMeaSeePromSelect =
                ENUM_ATTR_BACKUP_MEASUREMENT_SEEPROM_SELECT_SECONDARY;
        }

        // Set the Attribute as per bootcount policy for boot seeprom
        if (DT_SET_PROP(ATTR_BACKUP_SEEPROM_SELECT, procTarget,
                        bkpSeePromSelect))
        {
            log<level::ERR>(
                "Attribute [ATTR_BACKUP_SEEPROM_SELECT] set failed");
            throw std::runtime_error(
                "Attribute [ATTR_BACKUP_SEEPROM_SELECT] set failed");
        }

        // Set the Attribute as per bootcount policy for measurement seeprom
        if (DT_SET_PROP(ATTR_BACKUP_MEASUREMENT_SEEPROM_SELECT, procTarget,
                        bkpMeaSeePromSelect))
        {
            log<level::ERR>(
                "Attribute [ATTR_BACKUP_MEASUREMENT_SEEPROM_SELECT] set "
                "failed");
            throw std::runtime_error(
                "Attribute [ATTR_BACKUP_MEASUREMENT_SEEPROM_SELECT] set "
                "failed");
        }
    }
}

/**
 * @brief Read the HW Level from VPD and set CLK NE termination site
 * Note any failure in this function will result startHost failure.
 */
void setClkNETerminationSite()
{
    // Get Motherborad VINI Recored "HW" keyword
    constexpr auto objPath =
        "/xyz/openbmc_project/inventory/system/chassis/motherboard";
    constexpr auto kwdVpdInf = "com.ibm.ipzvpd.VINI";
    constexpr auto hwKwd = "HW";

    auto bus = sdbusplus::bus::new_default();

    std::string service = util::getService(bus, objPath, kwdVpdInf);

    auto properties = bus.new_method_call(
        service.c_str(), objPath, "org.freedesktop.DBus.Properties", "Get");
    properties.append(kwdVpdInf);
    properties.append(hwKwd);

    // Store "HW" Keyword data.
    std::variant<std::vector<uint8_t>> val;
    try
    {
        auto result = bus.call(properties);
        result.read(val);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ERR>("Get HW Keyword read from VINI Failed");
        throw std::runtime_error("Get HW Keyword read from VINI Failed");
    }

    auto hwData = std::get<std::vector<uint8_t>>(val);

    //"HW" Keyword size is 2 as per VPD spec.
    constexpr auto hwKwdSize = 2;
    if (hwKwdSize != hwData.size())
    {
        log<level::ERR>(
            fmt::format("Incorrect VINI records HW Keyword data size({})",
                        hwData.size())
                .c_str());
        throw std::runtime_error("Incorrect VINI records HW Keyword data size");
    }

    log<level::DEBUG>(fmt::format("VINI Records HW[0]:{} HW[1]:{}",
                                  hwData.at(0), hwData.at(1))
                          .c_str());

    // VINI Record "HW" keyword's Byte 0's MSB bit indicates
    // proc or planar type need to choose.
    constexpr uint8_t SYS_CLK_NE_TERMINATION_ON_MASK = 0x80;

    ATTR_SYS_CLK_NE_TERMINATION_SITE_Type clockTerm =
        ENUM_ATTR_SYS_CLK_NE_TERMINATION_SITE_PLANAR;

    if (SYS_CLK_NE_TERMINATION_ON_MASK & hwData.at(0))
    {
        clockTerm = ENUM_ATTR_SYS_CLK_NE_TERMINATION_SITE_PROC;
    }

    // update all the processor attributes
    struct pdbg_target* procTarget;
    pdbg_for_each_class_target("proc", procTarget)
    {

        if (DT_SET_PROP(ATTR_SYS_CLK_NE_TERMINATION_SITE, procTarget,
                        clockTerm))
        {
            log<level::ERR>(
                "Attribute ATTR_SYS_CLK_NE_TERMINATION_SITE set failed");
            throw std::runtime_error(
                "Attribute ATTR_SYS_CLK_NE_TERMINATION_SITE set failed");
        }
    }
}

/**
 * @brief Starts the self boot engine on POWER processor position 0
 *        to kick off a boot.
 * @return void
 */
void startHost(enum ipl_type iplType = IPL_TYPE_NORMAL)
{
    try
    {
        phal_init();
        ipl_set_type(iplType);
    }
    catch (std::exception& ex)
    {
        log<level::ERR>("Exception raised during init PHAL",
                        entry("EXCEPTION=%s", ex.what()));
        openpower::pel::detail::processBootErrorCallback(false);
        throw std::runtime_error("PHAL initialization failed");
    }

    // To clear trace if success
    openpower::pel::detail::processBootErrorCallback(true);

    setClkNETerminationSite();

    // callback method will be called upon failure which will create the PEL
    int rc = ipl_run_major(0);
    if (rc > 0)
    {
        log<level::ERR>("step 0 failed to start the host");
        throw std::runtime_error("Failed to execute host start boot step");
    }
}

/**
 * @brief Starts the reboot with type memory preserving reboot.
 * @return void
 */
void startHostMpReboot()
{
    // set ipl type as mpipl
    startHost(IPL_TYPE_MPIPL);
}

/**
 * @brief Starts the normal boot type.
 * @return void
 */
void startHostNormal()
{
    // Run select seeprom before poweron
    try
    {
        selectBootSeeprom();

        // To clear trace as it is success
        openpower::pel::detail::processBootErrorCallback(true);
    }
    catch (const std::exception& ex)
    {
        // create PEL in failure
        openpower::pel::detail::processBootErrorCallback(false);
        log<level::ERR>("SEEPROM selection failed", entry("ERR=%s", ex.what()));
        throw ex;
    }

    startHost();
}

REGISTER_PROCEDURE("startHost", startHostNormal)
REGISTER_PROCEDURE("startHostMpReboot", startHostMpReboot)

} // namespace phal
} // namespace openpower
