#include <libpdbg.h>

#include <attn/attn_common.hpp>
#include <sdbusplus/bus.hpp>

#include <map>

namespace attn
{

/** @brief Transition the host state */
void transitionHost(const HostState i_hostState)
{
    // The host quiesce code will handle the instruction-stop task(s)
    // thread_stop_all(); // in libpdbg

    // We will be transitioning host by starting appropriate dbus target
    std::string target = "obmc-host-quiesce@0.target"; // quiesce is default

    // diagnostic mode state requested
    if (HostState::Diagnostic == i_hostState)
    {
        target = "obmc-host-diagnostic-mode@0.target";
    }

    auto bus    = sdbusplus::bus::new_system();
    auto method = bus.new_method_call(
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", "StartUnit");

    method.append(target);    // target unit to start
    method.append("replace"); // mode = replace conflicting queued jobs

    bus.call_noreply(method); // start the service
}

} // namespace attn
