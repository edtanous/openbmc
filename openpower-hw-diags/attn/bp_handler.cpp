#include <attn_logging.hpp>
#include <sdbusplus/bus.hpp>

namespace attn
{

/**
 * @brief Notify Cronus over dbus interface
 *
 * When the special attention is due to a breakpoint condition we will notify
 * Cronus over the dbus interface.
 */
void bpHandler()
{
    // trace message
    trace<level::INFO>("Notify Cronus");

    // notify Cronus over dbus
    auto bus = sdbusplus::bus::new_system();
    auto msg = bus.new_signal("/", "org.openbmc.cronus", "Brkpt");

    // Cronus will figure out proc, core, thread so just send 0,0,0
    std::array<uint32_t, 3> params{0, 0, 0};
    msg.append(params);

    msg.signal_send();

    return;
}

} // namespace attn
