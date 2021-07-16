#include "fw_update_watch.hpp"

#include <fmt/format.h>

#include <phosphor-logging/elog.hpp>
#include <sdbusplus/exception.hpp>

namespace openpower
{
namespace phal
{
namespace fwupdate
{
using namespace phosphor::logging;
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

        // Colect device tree data
        openpower::phal::fwupdate::exportDevtree();
    }

    return;
}

void exportDevtree()
{}

} // namespace fwupdate
} // namespace phal
} // namespace openpower
