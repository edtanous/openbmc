#include <attn/attn_common.hpp>
#include <attn/attn_logging.hpp>
#include <sdbusplus/bus.hpp>
#include <util/trace.hpp>

namespace attn
{

/**
 * @brief Notify Cronus over dbus interface
 *
 * When the special attention is due to a breakpoint condition we will notify
 * Cronus over the dbus interface.
 */
int bpHandler()
{
    int rc = RC_SUCCESS; // assume success

    // trace message
    trace::inf("Notify Cronus");

    // notify Cronus over dbus
    try
    {
        auto bus = sdbusplus::bus::new_system();
        auto msg = bus.new_signal("/", "org.openbmc.cronus", "Brkpt");

        // Cronus will figure out proc, core, thread so just send 0,0,0
        std::array<uint32_t, 3> params{0, 0, 0};
        msg.append(params);

        msg.signal_send();
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace::inf("bpHandler() exception");
        trace::err(e.what());
        rc = RC_NOT_HANDLED;
    }

    return rc;
}

} // namespace attn
