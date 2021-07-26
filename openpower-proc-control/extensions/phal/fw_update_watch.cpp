#include "config.h"

#include "fw_update_watch.hpp"

#include "common_utils.hpp"
#include "create_pel.hpp"
#include "pdbg_utils.hpp"

#include <fmt/format.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>
#include <sdbusplus/exception.hpp>

#include <filesystem>

namespace openpower
{
namespace phal
{
namespace fwupdate
{
using namespace phosphor::logging;
namespace fs = std::filesystem;

using Message = std::string;
using Attributes = std::variant<Message>;
using PropertyName = std::string;
using PropertyMap = std::map<PropertyName, Attributes>;
using InterfaceName = std::string;
using InterfaceMap = std::map<InterfaceName, PropertyMap>;

void Watch::fwIntfAddedCallback(sdbusplus::message::message& msg)
{
    //  DBusInterfaceAdded interfaces;
    sdbusplus::message::object_path objectPath;
    InterfaceMap interfaceMap;

    try
    {
        msg.read(objectPath, interfaceMap);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ERR>(fmt::format("Failed to parse software add signal"
                                    "Exception [{}] REPLY_SIG [{}]",
                                    e.what(), msg.get_signature())
                            .c_str());
        return;
    }

    auto iter = interfaceMap.find("xyz.openbmc_project.Software.Activation");
    if (iter == interfaceMap.end())
    {
        // Skip not related Software Activation
        return;
    }

    auto attr = iter->second.find("Activation");
    if (attr == iter->second.end())
    {
        // Skip not related to Activation property.
        return;
    }

    auto& imageProperty = std::get<InterfaceName>(attr->second);
    if (imageProperty.empty())
    {
        // Skip, no image property
        return;
    }

    if (imageProperty ==
            "xyz.openbmc_project.Software.Activation.Activations.Ready" &&
        !isSoftwareUpdateInProgress())
    {
        log<level::INFO>("Software path interface add signal received");

        // Set status to code update in progress.
        // Interface added signal triggered multiple times in code update path,
        // it's due to additional interfaces added by the software manager app
        // after the version interface is created.
        // Device tree data collection is required only for the first trigger
        setSoftwareUpdateProgress(true);

        try
        {
            // Colect device tree data
            openpower::phal::fwupdate::exportDevtree();
        }
        catch (fs::filesystem_error& e)
        {
            log<level::ERR>(
                fmt::format("Filesystem error reported Error:({})", e.what())
                    .c_str());
            throw std::runtime_error(e.what());
        }

        log<level::INFO>("Successfully exported devtree attribute data");
    }

    return;
}

void exportDevtree()
{
    constexpr auto ERROR_DEVTREE_BACKUP =
        "org.open_power.PHAL.Error.devtreeBackup";

    // Check devtree export filter file is present
    auto path = fs::path(DEVTREE_EXPORT_FILTER_FILE);
    if (!fs::exists(path))
    {
        log<level::ERR>(
            fmt::format("devtree export filter file is not available: ({})",
                        DEVTREE_EXPORT_FILTER_FILE)
                .c_str());
        openpower::pel::createPEL(ERROR_DEVTREE_BACKUP);
        return;
    }

    // delete export data file if present
    auto expFile = fs::path(DEVTREE_EXP_FILE);
    if (fs::exists(expFile))
    {
        fs::remove_all(expFile);
    }
    else
    {
        // create directory
        fs::create_directory(expFile.parent_path());
    }

    // Update PDBG_DTB value
    openpower::phal::setDevtreeEnv();

    int status = 0;
    pid_t pid = fork();
    if (pid == 0)
    {
        std::string cmd("/usr/bin/attributes ");
        cmd += "export ";
        cmd += DEVTREE_EXPORT_FILTER_FILE;
        cmd += " > ";
        cmd += DEVTREE_EXP_FILE;
        execl("/bin/sh", "sh", "-c", cmd.c_str(), 0);

        auto error = errno;
        log<level::ERR>(fmt::format("Error occurred during attributes function "
                                    "execution, errno({})",
                                    error)
                            .c_str());
        // Creating PEL at one place.
    }
    else if (pid > 0)
    {
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status))
        {
            log<level::ERR>("Failed to collect attribute export data");
            openpower::pel::createPEL(ERROR_DEVTREE_BACKUP);
        }
    }
    else
    {
        log<level::ERR>("exportDevtree fork() failed");
        std::runtime_error("exportDevtree fork() failed");
    }
}

} // namespace fwupdate
} // namespace phal
} // namespace openpower
