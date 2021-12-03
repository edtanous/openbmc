#include "config.h"

#include "extensions/phal/create_pel.hpp"
#include "registration.hpp"
#include "temporary_file.hpp"

#include <fcntl.h>
#include <fmt/format.h>

#include <nlohmann/json.hpp>
#include <phosphor-logging/elog-errors.hpp>

#include <cstdio>
#include <filesystem>

extern "C"
{
#include <dtree.h>
}

namespace openpower
{
namespace phal
{
using namespace phosphor::logging;
using FILE_Ptr = std::unique_ptr<FILE, decltype(&::fclose)>;
namespace fs = std::filesystem;

void applyAttrOverride(fs::path& devtreeFile)
{
    constexpr auto DEVTREE_ATTR_OVERRIDE_PATH = "/tmp/devtree_attr_override";
    auto overrideFile = fs::path(DEVTREE_ATTR_OVERRIDE_PATH);
    if (!fs::exists(overrideFile))
    {
        return;
    }

    // Open attribute override file in r/o mode
    FILE_Ptr fpOverride(fopen(DEVTREE_ATTR_OVERRIDE_PATH, "r"), fclose);

    // Update Devtree with attribute override data.
    auto ret = dtree_cronus_import(devtreeFile.c_str(), CEC_INFODB_PATH,
                                   fpOverride.get());
    if (ret)
    {
        log<level::ERR>(
            fmt::format("Failed({}) to update attribute override data", ret)
                .c_str());
        throw std::runtime_error(
            "applyAttrOverride: dtree_cronus_import failed");
    }
    log<level::INFO>("DEVTREE: Applied attribute override data");
}

/**
 * @brief reinitialize the devtree attributes.
 * In the regular host boot path devtree attribute need to
 * initialize the default data and also some of the selected
 * attributes need to preserve with previous boot value.
 * Preserve attribute list is available BMC pre-defined location.
 * This function helps to meet the host ipl requirement
 * related to attribute persistency management for host ipl.
 * Steps involved
 * 1. Create attribute data file from devtree r/w version based on
 *    the reinit attribute list file bmc /usr/share/pdata path.
 * 2. Create temporary devtree file by copying devtree r/o file
 * 3. Override temporary copy of devtree with attribute data file
 *    from step 1.
 * 3a. Apply user provided attribute override if present in the
 *     predefined location.
 * 4. Copy  temporary copy devtree to r/w devtree version file.
 */

void reinitDevtree()
{
    using json = nlohmann::json;

    log<level::INFO>("reinitDevtree: started");

    // All the file operations is done on temporary copy
    // This is to avoid any file corruption issue during
    // copy or attribute import path.
    openpower::util::TemporaryFile tmpDevtreeFile{};
    const auto copyOptions = std::filesystem::copy_options::overwrite_existing;
    auto tmpDevtreePath = tmpDevtreeFile.getPath();
    bool tmpReinitDone = false;
    // To store callouts details in json format as per pel expectation.
    json jsonCalloutDataList;
    jsonCalloutDataList = json::array();

    try
    {
        // Check devtree reinit attributes list file is present
        auto attrFile = fs::path(DEVTREE_REINIT_ATTRS_LIST);
        if (!fs::exists(attrFile))
        {
            log<level::ERR>(
                fmt::format(
                    "devtree attribute export list file is not available: ({})",
                    DEVTREE_REINIT_ATTRS_LIST)
                    .c_str());
            throw std::runtime_error("reinitDevtree: missing export list file");
        }

        // create temporary data file to store the devtree export data
        openpower::util::TemporaryFile tmpFile{};

        {
            // get temporary datafile pointer.
            FILE_Ptr fpExport(fopen(tmpFile.getPath().c_str(), "w+"), fclose);

            if (fpExport.get() == nullptr)
            {
                log<level::ERR>(
                    fmt::format("Temporary data file failed to open: ({})",
                                tmpFile.getPath().c_str())
                        .c_str());
                throw std::runtime_error(
                    "reinitDevtree: failed to open temporaray data file");
            }

            // Step 1: export devtree data based on the reinit attribute list.
            auto ret =
                dtree_cronus_export(CEC_DEVTREE_RW_PATH, CEC_INFODB_PATH,
                                    DEVTREE_REINIT_ATTRS_LIST, fpExport.get());
            if (ret)
            {
                log<level::ERR>(
                    fmt::format("Failed({}) to collect attribute export data",
                                ret)
                        .c_str());
                throw std::runtime_error(
                    "reinitDevtree: dtree_cronus_export function failed");
            }
        }

        // Step 2: Create temporary devtree file by copying devtree r/o version
        std::filesystem::copy(CEC_DEVTREE_RO_PATH, tmpDevtreePath, copyOptions);

        // get r/o version data file pointer
        FILE_Ptr fpImport(fopen(tmpFile.getPath().c_str(), "r"), fclose);
        if (fpImport.get() == nullptr)
        {
            log<level::ERR>(
                fmt::format("import, temporary data file failed to open: ({})",
                            tmpFile.getPath().c_str())
                    .c_str());
            throw std::runtime_error(
                "reinitDevtree: import, failed to open temporaray data file");
        }

        // Step 3: Update Devtree r/w version with data file attribute data.
        auto ret = dtree_cronus_import(tmpDevtreePath.c_str(), CEC_INFODB_PATH,
                                       fpImport.get());
        if (ret)
        {
            log<level::ERR>(
                fmt::format("Failed({}) to update attribute data", ret)
                    .c_str());
            throw std::runtime_error(
                "reinitDevtree: dtree_cronus_import function failed");
        }
        // Step 3.a: Apply user provided attribute override data if present.
        applyAttrOverride(tmpDevtreePath);

        // Temporary file reinit is success.
        tmpReinitDone = true;
    }
    catch (const std::exception& e)
    {
        // Any failures during temporary file re-init should create PEL
        // and continue with current version of devtree file to allow boot.
        log<level::ERR>(
            fmt::format("reinitDevtree failed ({})", e.what()).c_str());
        json jsonCalloutDataList;
        jsonCalloutDataList = json::array();
        json jsonCalloutData;
        jsonCalloutData["Procedure"] = "BMC0001";
        jsonCalloutData["Priority"] = "M";
        jsonCalloutDataList.emplace_back(jsonCalloutData);
        openpower::pel::createErrorPEL(
            "org.open_power.PHAL.Error.devtreeReinit", jsonCalloutDataList);
    }

    // Step 4: Update devtree r/w file
    try
    {
        if (tmpReinitDone)
        {
            // Step 4: Copy temporary version devtree file r/w version file.
            // Any copy failures should results service failure.
            std::filesystem::copy(tmpDevtreePath, CEC_DEVTREE_RW_PATH,
                                  copyOptions);
            log<level::INFO>("reinitDevtree: completed successfully");
        }
        else
        {
            // Attempt boot with genesis mode attribute data.
            log<level::WARNING>("reinitDevtree: DEVTREE(r/w) initilizing with "
                                "genesis mode attribute data");
            std::filesystem::copy(CEC_DEVTREE_RO_PATH, CEC_DEVTREE_RW_PATH,
                                  copyOptions);
        }
    }
    catch (const std::exception& e)
    {
        // Any failures during update on r/w file is serious error should create
        // PEL, with code callout. Also failed the service.
        // and continue with current version of devtree file to allow boot.
        log<level::ERR>(
            fmt::format("reinitDevtree r/w version update failed ({})",
                        e.what())
                .c_str());
        json jsonCalloutDataList;
        jsonCalloutDataList = json::array();
        json jsonCalloutData;
        jsonCalloutData["Procedure"] = "BMC0001";
        jsonCalloutData["Priority"] = "H";
        jsonCalloutDataList.emplace_back(jsonCalloutData);
        openpower::pel::createErrorPEL(
            "org.open_power.PHAL.Error.devtreeReinit", jsonCalloutDataList);
        throw;
    }
}

REGISTER_PROCEDURE("reinitDevtree", reinitDevtree)

} // namespace phal
} // namespace openpower
