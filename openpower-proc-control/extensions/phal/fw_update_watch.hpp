#pragma once

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>

namespace openpower
{
namespace phal
{
namespace fwupdate
{

static constexpr auto OBJ_SOFTWARE = "/xyz/openbmc_project/software";

/** @class Watch
 *  @brief Adds d-bus signal based watch for software path interface add.
 *  @details This implements methods for watching for software path
 *  interface add signal and call appropriate function to initiate phal
 *  devtree attribute data collection and save to preserve partition.
 *  Rules:
 *   - Watch for interfaces added for the software path
 *   - If interface added is “Activation”
 *   - if Activation property value is “Ready”
 *   - Then software update is going to start
 *   - Collect phal devtree required attribute list and save to
 *     pre-defined location
 *
 */
class Watch
{
  public:
    Watch() = delete;
    ~Watch() = default;
    Watch(const Watch&) = delete;
    Watch& operator=(const Watch&) = delete;
    Watch(Watch&&) = default;
    Watch& operator=(Watch&&) = default;

    /** @brief constructs watch for interface add signals.
     *  @param[in] bus -  The Dbus bus object
     */

    Watch(sdbusplus::bus::bus& bus) :
        addMatch(bus,
                 sdbusplus::bus::match::rules::interfacesAdded() +
                     sdbusplus::bus::match::rules::path(OBJ_SOFTWARE),
                 std::bind(std::mem_fn(&Watch::fwIntfAddedCallback), this,
                           std::placeholders::_1))
    {}

  private:
    /** @brief Method to check whether software update is in progress
     *  @return - bool
     */
    bool isSoftwareUpdateInProgress() const
    {
        return softwareUpdateInProgress;
    }

    /** @brief Method to indicate whether software update is in progress
     *
     *  @param[in] progress
     *
     */
    void setSoftwareUpdateProgress(bool progress)
    {
        softwareUpdateInProgress = progress;
    }

    /** @brief Callback function for software path add signal.
     *  @details Function provide required data related to new
     *  software path interface added to see software update progress.
     *
     *  @param[in] msg  - Data associated with subscribed signal
     */
    void fwIntfAddedCallback(sdbusplus::message::message& msg);

    /** @brief sdbusplus signal match for software path add */
    sdbusplus::bus::match_t addMatch;

    /** @brief indicates whether software update is going on */
    bool softwareUpdateInProgress = false;
};

/** @brief function to export phal devtree data file based
 * on the filter file,
 */
void exportDevtree();

} // namespace fwupdate
} // namespace phal
} // namespace openpower
