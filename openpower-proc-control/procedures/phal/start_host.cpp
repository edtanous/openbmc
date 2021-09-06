extern "C"
{
#include <libpdbg.h>
}

#include "attributes_info.H"

#include "extensions/phal/common_utils.hpp"
#include "extensions/phal/create_pel.hpp"
#include "extensions/phal/phal_error.hpp"
#include "util.hpp"

#include <fmt/format.h>
#include <libekb.H>

#include <ext_interface.hpp>
#include <nlohmann/json.hpp>
#include <phosphor-logging/log.hpp>
#include <registration.hpp>

namespace openpower
{
namespace phal
{

using namespace phosphor::logging;

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
        if (!isPrimaryProc(procTarget))
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
    catch (const sdbusplus::exception::exception& e)
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
 * @brief Helper function to create error log (aka PEL) with
 *        procedure callout for the hardware isolation policy
 *        settings failures.
 *
 * @param[in] procedureCode - The procedure code to include in the callout
 * @param[in] priority - The priority for the procedure callout
 * @param[in] additionalData - The additional data to include in the error log
 *
 * @return void
 */
static void
    createPELForHwIsolationSettingsErr(const std::string& procedureCode,
                                       const std::string& priority,
                                       const pel::FFDCData& additionalData)
{
    try
    {
        using json = nlohmann::json;

        json jsonCalloutDataList;
        jsonCalloutDataList = json::array();
        json jsonCalloutData;
        jsonCalloutData["Procedure"] = procedureCode;
        jsonCalloutData["Priority"] = priority;
        jsonCalloutDataList.emplace_back(jsonCalloutData);

        openpower::pel::createErrorPEL("org.open_power.PHAL.Error.Boot",
                                       jsonCalloutDataList, additionalData);
    }
    catch (const std::exception& e)
    {
        // Don't throw exception since the caller might call in the error path
        // and even we should allow the hardware isolation by default.
        log<level::ERR>(
            fmt::format("Exception [{}], failed to create the error log "
                        "for the hardware isolation policy settings failures.",
                        e.what())
                .c_str());
    }
}

/**
 * @brief Helper function to decide the hardware isolation (aka guard)
 *
 * @return xyz.openbmc_project.Object.Enable::Enabled value on success
 *         true on failure since hardware isolation feature should be
 *         enabled by default.
 */
static bool allowHwIsolation()
{
    bool allowHwIsolation{true};

    constexpr auto hwIsolationPolicyObjPath =
        "/xyz/openbmc_project/hardware_isolation/allow_hw_isolation";
    constexpr auto hwIsolationPolicyIface = "xyz.openbmc_project.Object.Enable";

    try
    {
        auto bus = sdbusplus::bus::new_default();

        std::string service = util::getService(bus, hwIsolationPolicyObjPath,
                                               hwIsolationPolicyIface);

        auto method =
            bus.new_method_call(service.c_str(), hwIsolationPolicyObjPath,
                                "org.freedesktop.DBus.Properties", "Get");
        method.append(hwIsolationPolicyIface, "Enabled");

        auto reply = bus.call(method);

        std::variant<bool> resp;

        reply.read(resp);

        if (const bool* enabledPropVal = std::get_if<bool>(&resp))
        {
            allowHwIsolation = *enabledPropVal;
        }
        else
        {
            const auto trace{fmt::format(
                "Failed to read the HardwareIsolation policy "
                "from the path [{}] interface [{}]. Continuing with "
                "default mode(allow_hw_isolation)",
                hwIsolationPolicyObjPath, hwIsolationPolicyIface)};

            log<level::ERR>(trace.c_str());
            createPELForHwIsolationSettingsErr("BMC0001", "M",
                                               {{"REASON_FOR_PEL", trace}});
        }
    }
    catch (const sdbusplus::exception::exception& e)
    {
        const auto trace{fmt::format(
            "Exception [{}] to get the HardwareIsolation policy "
            "from the path [{}] interface [{}]. Continuing with "
            "default mode (allow_hw_isolation)",
            e.what(), hwIsolationPolicyObjPath, hwIsolationPolicyIface)};

        log<level::ERR>(trace.c_str());
        createPELForHwIsolationSettingsErr("BMC0001", "M",
                                           {{"REASON_FOR_PEL", trace}});
    }

    return allowHwIsolation;
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

        /**
         * Don't apply guard records if the HardwareIsolation (aka guard)
         * the policy is disabled (false). By default, libipl will apply
         * guard records.
         */
        if (!allowHwIsolation())
        {
            ipl_disable_guard();
        }

        if (iplType == IPL_TYPE_NORMAL)
        {
            // Update SEEPROM side only for NORMAL boot
            selectBootSeeprom();
        }
        setClkNETerminationSite();
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>("Exception raised during ipl initialisation",
                        entry("EXCEPTION=%s", ex.what()));
        openpower::pel::createPEL("org.open_power.PHAL.Error.Boot");
        openpower::pel::detail::processBootError(false);
        throw std::runtime_error("IPL initialization failed");
    }

    // To clear trace if success
    openpower::pel::detail::processBootError(true);

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
    startHost(IPL_TYPE_NORMAL);
}

REGISTER_PROCEDURE("startHost", startHostNormal)
REGISTER_PROCEDURE("startHostMpReboot", startHostMpReboot)

} // namespace phal
} // namespace openpower
