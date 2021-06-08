#pragma once

#include <sdbusplus/bus.hpp>

#include <string>

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
} // namespace openpower
