#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <attn/ti_handler.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>

namespace attn
{

/** @brief Start host diagnostic mode or quiesce host on TI */
int tiHandler(TiDataArea* i_tiDataArea)
{
    int rc = RC_NOT_HANDLED; // assume TI not handled

    if (0xa1 == i_tiDataArea->command)
    {
        // Use the systemd service manager object interface to call the
        // start unit method with the obmc-host-diagnostic-mode target.
        auto bus    = sdbusplus::bus::new_system();
        auto method = bus.new_method_call(
            "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
            "org.freedesktop.systemd1.Manager", "StartUnit");

        if (autoRebootEnabled())
        {
            // If autoreboot is enabled we will start diagnostic mode target
            // which will ultimately mpipl the host.
            trace<level::INFO>("start obmc-host-diagnostic-mode target");
            method.append("obmc-host-diagnostic-mode@0.target");
        }
        else
        {
            // If autoreboot is disabled we will start the host quiesce target
            trace<level::INFO>("start obmc-host-quiesce target");
            method.append("obmc-host-quiesce@0.target");
        }
        method.append("replace"); // mode = replace conflicting queued jobs
        bus.call_noreply(method); // start the service

        rc = RC_SUCCESS;
    }

    return rc;
}

/** @brief Read autoreboot property **/
bool autoRebootEnabled()
{
    // Use dbus get-property interface to read the autoreboot property
    auto bus = sdbusplus::bus::new_system();
    auto method =
        bus.new_method_call("xyz.openbmc_project.Settings",
                            "/xyz/openbmc_project/control/host0/auto_reboot",
                            "org.freedesktop.DBus.Properties", "Get");
    method.append("xyz.openbmc_project.Control.Boot.RebootPolicy",
                  "AutoReboot");
    try
    {
        auto reply = bus.call(method);
        std::variant<bool> result;
        reply.read(result);
        auto autoReboot = std::get<bool>(result);
        if (autoReboot)
        {
            trace<level::INFO>("Auto reboot enabled");
            return true;
        }
        else
        {
            trace<level::INFO>("Auto reboot disabled.");
            return false;
        }
    }
    catch (const sdbusplus::exception::SdBusError& ec)
    {
        // Error reading autoreboot
        std::string traceMessage =
            "Error in AutoReboot Get: " + std::string(ec.what());
        trace<level::INFO>(traceMessage.c_str());
        return false; // assume autoreboot disabled
    }
}
} // namespace attn
