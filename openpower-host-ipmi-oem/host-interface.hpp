#pragma once

#include <ipmid-host/cmd-utils.hpp>
#include <org/open_power/Control/Host/server.hpp>
#include <sdbusplus/bus.hpp>
namespace open_power
{
namespace host
{
namespace command
{

namespace Base = sdbusplus::org::open_power::Control::server;
using IpmiCmdData = phosphor::host::command::IpmiCmdData;

/** @class Host
 *  @brief OpenBMC control host interface implementation.
 *  @details A concrete implementation for org.open_power.Control.Host
 *  DBus API.
 */
class Host : public sdbusplus::server::object::object<Base::Host>
{
  public:
    /** @brief Constructs Host Control Interface
     *
     *  @param[in] bus     - The Dbus bus object
     *  @param[in] objPath - The Dbus object path
     */
    Host(sdbusplus::bus::bus& bus, const char* objPath) :
        sdbusplus::server::object::object<Base::Host>(bus, objPath), bus(bus)
    {
        // Nothing to do
    }

    /** @brief Sends input command to host
     *         Note that the command will be queued in a FIFO if
     *         other commands to the host have yet to be run
     *
     *  @param[in] command - Input command to execute
     *  @param[in] data    - Data associated with the command
     */
    void execute(Command command, std::variant<uint8_t> data) override;

  private:
    /** @brief sdbusplus DBus bus connection. */
    sdbusplus::bus::bus& bus;

    /** @brief  Callback function to be invoked by command manager
     *
     *  @detail Conveys the status of the last Host bound command.
     *          Depending on the status,  a CommandComplete or
     *          CommandFailure signal would be sent
     *
     *  @param[in] cmd    - IPMI command and data sent to Host
     *  @param[in] status - Success or Failure
     */
    void commandStatusHandler(IpmiCmdData cmd, bool status);
};

} // namespace command
} // namespace host
} // namespace open_power
