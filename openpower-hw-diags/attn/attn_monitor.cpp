#include <attn/attn_handler.hpp>
#include <attn/attn_monitor.hpp>
#include <util/trace.hpp>

namespace attn
{

/** @brief Register a callback for gpio event */
void AttnMonitor::scheduleGPIOEvent()
{
    // Register async callback, note that callback is a
    // lambda function with "this" pointer captured
    iv_gpioEventDescriptor.async_wait(
        boost::asio::posix::stream_descriptor::wait_read,
        [this](const boost::system::error_code& ec) {
            if (ec)
            {
                trace::err("GPIO Async wait error: %s", ec.message().c_str());
            }
            else
            {
                trace::inf("Attention GPIO active");
                handleGPIOEvent(); // gpio trigger detected
            }
            return;
        }); // register async callback
}

/** @brief Handle the GPIO state change event */
void AttnMonitor::handleGPIOEvent()
{
    gpiod_line_event gpioEvent;

    if (gpiod_line_event_read_fd(iv_gpioEventDescriptor.native_handle(),
                                 &gpioEvent) < 0)
    {
        trace::err("GPIO line read failed");
    }
    else
    {
        switch (gpiod_line_get_value(iv_gpioLine))
        {
            // active attention when gpio == 0
            case 0:
                attnHandler(iv_config);
                break;

            // gpio == 1, GPIO handler should not be executing
            case 1:
                trace::inf("GPIO handler out of sync");
                break;

            // unexpected value
            default:
                trace::inf("GPIO line unexpected value");
        }
    }
    scheduleGPIOEvent(); // continue monitoring gpio
}

/** @brief Request a GPIO line for monitoring attention events */
void AttnMonitor::requestGPIOEvent()
{
    if (0 != gpiod_line_request(iv_gpioLine, &iv_gpioConfig, 0))
    {
        trace::err("failed request for GPIO");
    }
    else
    {
        int gpioLineFd;

        gpioLineFd = gpiod_line_event_get_fd(iv_gpioLine);
        if (gpioLineFd < 0)
        {
            trace::err("failed to get file descriptor");
        }
        else
        {
            // Register file descriptor for monitoring
            iv_gpioEventDescriptor.assign(gpioLineFd);

            // Start monitoring
            scheduleGPIOEvent();
        }
    }
}

} // namespace attn
