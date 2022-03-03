#include <attn/attn_dbus.hpp>
#include <attn/attn_dump.hpp>
#include <attn/attn_logging.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <util/trace.hpp>

constexpr uint64_t dumpTimeout = 3600000000; // microseconds

constexpr auto operationStatusInProgress =
    "xyz.openbmc_project.Common.Progress.OperationStatus.InProgress";

namespace attn
{

/**
 *  Callback for dump request properties change signal monitor
 *
 * @param[in] i_msg         Dbus message from the dbus match infrastructure
 * @param[out] o_dumpStatus Dump status dbus response string
 * @return Always non-zero indicating no error, no cascading callbacks
 */
uint dumpStatusChanged(sdbusplus::message::message& i_msg,
                       std::string& o_dumpStatus)
{
    // reply (msg) will be a property change message
    std::string interface;
    std::map<std::string, std::variant<std::string, uint8_t>> property;
    i_msg.read(interface, property);

    // looking for property Status changes
    std::string propertyType = "Status";
    auto dumpStatus          = property.find(propertyType);

    if (dumpStatus != property.end())
    {
        const std::string* status =
            std::get_if<std::string>(&(dumpStatus->second));

        if (nullptr != status)
        {
            o_dumpStatus = *status;
        }
    }

    return 1; // non-negative return code for successful callback
}

/**
 * Register a callback for dump progress status changes
 *
 * @param[in] i_path The object path of the dump to monitor
 */
void monitorDump(const std::string& i_path)
{
    // setup the signal match rules and callback
    std::string matchInterface = "xyz.openbmc_project.Common.Progress";
    auto bus                   = sdbusplus::bus::new_system();

    // monitor dump status change property, will update dumpStatus
    std::string dumpStatus = "requested";
    std::unique_ptr<sdbusplus::bus::match_t> match =
        std::make_unique<sdbusplus::bus::match_t>(
            bus,
            sdbusplus::bus::match::rules::propertiesChanged(
                i_path.c_str(), matchInterface.c_str()),
            [&](auto& msg) { return dumpStatusChanged(msg, dumpStatus); });

    // wait for dump status to be completed (complete == true)
    trace::inf("dump requested %s", i_path.c_str());

    // wait for dump status not InProgress or timeout
    uint64_t timeRemaining = dumpTimeout;

    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    while (("requested" == dumpStatus ||
            operationStatusInProgress == dumpStatus) &&
           0 != timeRemaining)
    {
        bus.wait(timeRemaining);
        uint64_t timeElapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - begin)
                .count();

        timeRemaining =
            timeElapsed > timeRemaining ? 0 : timeRemaining - timeElapsed;

        bus.process_discard();
    }

    if (0 == timeRemaining)
    {
        trace::err("dump request timed out after %" PRIu64 " microseconds",
                   dumpTimeout);
    }

    trace::inf("dump status: %s", dumpStatus.c_str());
}

/** Request a dump from the dump manager */
void requestDump(uint32_t i_logId, const DumpParameters& i_dumpParameters)
{
    constexpr auto path      = "/org/openpower/dump";
    constexpr auto interface = "xyz.openbmc_project.Dump.Create";
    constexpr auto function  = "CreateDump";

    sdbusplus::message::message method;

    if (0 == dbusMethod(path, interface, function, method))
    {
        try
        {
            // dbus call arguments
            std::map<std::string, std::variant<std::string, uint64_t>>
                createParams;
            createParams["com.ibm.Dump.Create.CreateParameters.ErrorLogId"] =
                uint64_t(i_logId);
            if (DumpType::Hostboot == i_dumpParameters.dumpType)
            {
                createParams["com.ibm.Dump.Create.CreateParameters.DumpType"] =
                    "com.ibm.Dump.Create.DumpType.Hostboot";
            }
            else if (DumpType::Hardware == i_dumpParameters.dumpType)
            {
                createParams["com.ibm.Dump.Create.CreateParameters.DumpType"] =
                    "com.ibm.Dump.Create.DumpType.Hardware";
                createParams
                    ["com.ibm.Dump.Create.CreateParameters.FailingUnitId"] =
                        i_dumpParameters.unitId;
            }
            else if (DumpType::SBE == i_dumpParameters.dumpType)
            {
                createParams["com.ibm.Dump.Create.CreateParameters.DumpType"] =
                    "com.ibm.Dump.Create.DumpType.SBE";
                createParams
                    ["com.ibm.Dump.Create.CreateParameters.FailingUnitId"] =
                        i_dumpParameters.unitId;
            }
            method.append(createParams);

            // using system dbus
            auto bus      = sdbusplus::bus::new_system();
            auto response = bus.call(method);

            // reply will be type dbus::ObjectPath
            sdbusplus::message::object_path reply;
            response.read(reply);

            // monitor dump progress
            monitorDump(reply);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            trace::err("requestDump exception");
            trace::err(e.what());
        }
    }
}

} // namespace attn
