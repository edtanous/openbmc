#include <logging.hpp>
#include <sdbusplus/bus.hpp>

namespace attn
{

/** @brief Start host diagnostic mode systemd unit */
void tiHandler()
{
    // trace message
    log<level::INFO>("start host diagnostic mode service");

    // Use the systemd service manager object interface to call the start unit
    // method with the obmc-host-diagnostic-mode target.
    auto bus    = sdbusplus::bus::new_system();
    auto method = bus.new_method_call(
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", "StartUnit");

    method.append("obmc-host-diagnostic-mode@0.target"); // unit to activate
    method.append("replace"); // mode = replace conflicting queued jobs
    bus.call_noreply(method); // start the service

    return;
}

} // namespace attn
