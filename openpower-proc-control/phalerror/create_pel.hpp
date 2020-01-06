#pragma once

#include <sdbusplus/bus.hpp>
#include <string>
#include <vector>
namespace openpower
{
namespace util
{
/**
 * Get D-Bus service name for the specified object and interface
 *
 * @param[in] bus - sdbusplus D-Bus to attach to
 * @param[in] objectPath - D-Bus object path
 * @param[in] interface - D-Bus interface name
 *
 * @return service name on success and exception on failure
 */
std::string getService(sdbusplus::bus::bus& bus, const std::string& objectPath,
                       const std::string& interface);
} // namespace util
namespace pel
{
using FFDCData = std::vector<std::pair<std::string, std::string>>;
/**
 * Create boot error PEL
 *
 * @param[in] ffdcData - failure data to append to PEL
 */
void createBootErrorPEL(const FFDCData& ffdcData);

} // namespace pel
} // namespace openpower
