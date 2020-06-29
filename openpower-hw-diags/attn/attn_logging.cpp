#include <unistd.h>

#include <attn/attn_logging.hpp>
#include <phosphor-logging/log.hpp>

namespace attn
{

/** @brief journal entry of type INFO using phosphor logging */
template <>
void trace<INFO>(const char* i_message)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(i_message);
}

/** @brief add an event to the log for PEL generation */
void event(EventType i_event, std::map<std::string, std::string>& i_additional)
{
    bool eventValid = false; // assume no event created

    std::string eventName;

    switch (i_event)
    {
        case EventType::Checkstop:
            eventName  = "org.open_power.HwDiags.Error.Checkstop";
            eventValid = true;
            break;
        case EventType::Terminate:
            eventName  = "org.open_power.Attn.Error.Terminate";
            eventValid = true;
            break;
        case EventType::Vital:
            eventName  = "org.open_power.Attn.Error.Vital";
            eventValid = true;
            break;
        case EventType::HwDiagsFail:
            eventName  = "org.open_power.HwDiags.Error.Fail";
            eventValid = true;
            break;
        case EventType::AttentionFail:
            eventName  = "org.open_power.Attn.Error.Fail";
            eventValid = true;
            break;
        default:
            eventValid = false;
            break;
    }

    if (true == eventValid)
    {
        // Get access to logging interface and method for creating log
        auto bus = sdbusplus::bus::new_default_system();

        // using direct create method (for additional data)
        auto method = bus.new_method_call(
            "xyz.openbmc_project.Logging", "/xyz/openbmc_project/logging",
            "xyz.openbmc_project.Logging.Create", "Create");

        // attach additional data
        method.append(eventName,
                      "xyz.openbmc_project.Logging.Entry.Level.Error",
                      i_additional);

        // log the event
        auto reply = bus.call(method);
    }
}

/** @brief commit checkstop event to log */
void eventCheckstop(std::map<std::string, std::string>& i_errors)
{
    std::map<std::string, std::string> additionalData;

    // TODO need multi-error/multi-callout stuff here

    // if analyzer isolated errors
    if (!(i_errors.empty()))
    {
        // FIXME TEMP CODE - begin

        std::string signature = i_errors.begin()->first;
        std::string chip      = i_errors.begin()->second;

        additionalData["_PID"]      = std::to_string(getpid());
        additionalData["SIGNATURE"] = signature;
        additionalData["CHIP_ID"]   = chip;

        // FIXME TEMP CODE -end

        event(EventType::Checkstop, additionalData);
    }
}

/** @brief commit special attention TI event to log */
void eventTerminate()
{
    std::map<std::string, std::string> additionalData;

    additionalData["_PID"] = std::to_string(getpid());

    event(EventType::Terminate, additionalData);
}

/** @brief commit SBE vital event to log */
void eventVital()
{
    std::map<std::string, std::string> additionalData;

    additionalData["_PID"] = std::to_string(getpid());

    event(EventType::Vital, additionalData);
}

/** @brief commit analyzer failure event to log */
void eventHwDiagsFail(int i_error)
{
    std::map<std::string, std::string> additionalData;

    additionalData["_PID"] = std::to_string(getpid());

    event(EventType::HwDiagsFail, additionalData);
}

/** @brief commit attention handler failure event to log */
void eventAttentionFail(int i_error)
{
    std::map<std::string, std::string> additionalData;

    additionalData["_PID"]       = std::to_string(getpid());
    additionalData["ERROR_CODE"] = std::to_string(i_error);

    event(EventType::AttentionFail, additionalData);
}

} // namespace attn
