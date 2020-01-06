#include "create_pel.hpp"

#include <libekb.H>
#include <unistd.h>

#include <map>
#include <phosphor-logging/elog.hpp>
#include <xyz/openbmc_project/Logging/Entry/server.hpp>
namespace openpower
{
using namespace phosphor::logging;

namespace util
{
std::string getService(sdbusplus::bus::bus& bus, const std::string& objectPath,
                       const std::string& interface)
{
    constexpr auto mapperBusBame = "xyz.openbmc_project.ObjectMapper";
    constexpr auto mapperObjectPath = "/xyz/openbmc_project/object_mapper";
    constexpr auto mapperInterface = "xyz.openbmc_project.ObjectMapper";
    std::vector<std::pair<std::string, std::vector<std::string>>> response;
    auto method = bus.new_method_call(mapperBusBame, mapperObjectPath,
                                      mapperInterface, "GetObject");
    method.append(objectPath, std::vector<std::string>({interface}));
    try
    {
        auto reply = bus.call(method);
        reply.read(response);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ERR>("D-Bus call exception",
                        entry("OBJPATH=%s", mapperObjectPath),
                        entry("INTERFACE=%s", mapperInterface),
                        entry("EXCEPTION=%s", e.what()));

        throw std::runtime_error("Service name is not found");
    }

    if (response.empty())
    {
        throw std::runtime_error("Service name response is empty");
    }
    return response.begin()->first;
}
} // namespace util

namespace pel
{
void createBootErrorPEL(const FFDCData& ffdcData)
{
    constexpr auto loggingObjectPath = "/xyz/openbmc_project/logging";
    constexpr auto loggingInterface = "xyz.openbmc_project.Logging.Create";

    std::map<std::string, std::string> additionalData;
    auto bus = sdbusplus::bus::new_default();
    additionalData.emplace("_PID", std::to_string(getpid()));
    for (auto& data : ffdcData)
    {
        additionalData.emplace(data);
    }
    try
    {
        static constexpr auto bootErrorMessage =
            "org.open_power.PHAL.Error.Boot";
        std::string service =
            util::getService(bus, loggingObjectPath, loggingInterface);
        auto method = bus.new_method_call(service.c_str(), loggingObjectPath,
                                          loggingInterface, "Create");
        auto level =
            sdbusplus::xyz::openbmc_project::Logging::server::convertForMessage(
                sdbusplus::xyz::openbmc_project::Logging::server::Entry::Level::
                    Error);
        method.append(bootErrorMessage, level, additionalData);
        auto resp = bus.call(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ERR>("D-Bus call exception",
                        entry("OBJPATH=%s", loggingObjectPath),
                        entry("INTERFACE=%s", loggingInterface),
                        entry("EXCEPTION=%s", e.what()));

        throw std::runtime_error(
            "Error in invoking D-Bus logging create interface");
    }
}
} // namespace pel
} // namespace openpower
