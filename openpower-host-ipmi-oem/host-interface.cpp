#include <chrono>
#include <functional>
#include <oemhandler.hpp>
#include <host-ipmid/ipmid-host-cmd.hpp>
#include <host-ipmid/ipmid-host-cmd-utils.hpp>
#include <phosphor-logging/log.hpp>
#include <config.h>
#include <host-interface.hpp>
namespace open_power
{
namespace host
{
namespace command
{

// IPMI command
// https://github.com/openbmc/openbmc/issues/2082 for handling
// Non-OEM commands that need to send SMS_ATN
using OEMCmd = uint8_t;

// Map of IPMI OEM command to its equivalent interface command.
// This is needed when invoking the callback handler to indicate
// the status of the executed command.
static const std::map<OEMCmd, Host::Command> intfCommand = {
    {
        IPMI_CMD_OCC_RESET,
            Base::Host::Command::OCCReset
    }
};

// Called at user request
void Host::execute(Base::Host::Command command,
        sdbusplus::message::variant<uint8_t> data)
{
    using namespace phosphor::logging;

    log<level::INFO>("Pushing cmd on to queue",
            entry("CONTROL_HOST_CMD=%s",
                  convertForMessage(command)));

    // If the command is OCCReset, then all we need is just sensor ID
    // This is the only command that is being used now.
    if (command == Base::Host::Command::OCCReset)
    {
        auto sensorID = sdbusplus::message::variant_ns::get<uint8_t>(data);

        auto cmd = std::make_tuple(std::make_pair(IPMI_CMD_OCC_RESET, sensorID),
                std::bind(&Host::commandStatusHandler,
                    this, std::placeholders::_1,
                    std::placeholders::_2));

        return ipmid_send_cmd_to_host(std::move(cmd));
    }
    return;
}

// Called into by Command Manager
void Host::commandStatusHandler(IpmiCmdData cmd, bool status)
{
    // Need to convert <cmd> to the equivalent one mentioned in spec
    auto value = status ? Result::Success : Result::Failure;

    // Fire a signal
    this->commandComplete(intfCommand.at(std::get<0>(cmd)), value);
}

} // namespace command
} // namespace host
} // namepsace open_power
