#include <attn_handler.hpp>
#include <attn_logging.hpp>
#include <sdbusplus/bus.hpp>
#include <ti_handler.hpp>

namespace attn
{

/** @brief Start host diagnostic mode systemd unit */
int tiHandler(TiDataArea* i_tiDataArea)
{
    int rc = RC_NOT_HANDLED; // assume TI not handled

    if (0xa1 == i_tiDataArea->command)
    {
        // trace message
        trace<level::INFO>("start host diagnostic mode service");

        // Use the systemd service manager object interface to call the start
        // unit method with the obmc-host-diagnostic-mode target.
        auto bus    = sdbusplus::bus::new_system();
        auto method = bus.new_method_call(
            "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
            "org.freedesktop.systemd1.Manager", "StartUnit");

        method.append("obmc-host-diagnostic-mode@0.target"); // unit to activate
        method.append("replace"); // mode = replace conflicting queued jobs
        bus.call_noreply(method); // start the service

        rc = RC_SUCCESS;
    }

    return rc;
}

} // namespace attn
