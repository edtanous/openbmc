#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <attn/attn_monitor.hpp>

namespace attn
{

/** @brief Register a callback for gpio event */
void AttnMonitor::scheduleGPIOEvent()
{
    std::string logMessage = "... waiting for events ...";
    trace<level::INFO>(logMessage.c_str());

    // Register async callback, note that callback is a
    // lambda function with "this" pointer captured
    iv_gpioEventDescriptor.async_wait(
        boost::asio::posix::stream_descriptor::wait_read,
        [this](const boost::system::error_code& ec) {
            if (ec)
            {
                std::string logMessage =
                    "GPIO Async wait error: " + std::string(ec.message());
                trace<level::INFO>(logMessage.c_str());
            }
            else
            {
                trace<level::INFO>(
                    "Attention monitor detected active attention");
                handleGPIOEvent(); // gpio trigger detected
            }
            return;
        }); // register async callback
}

/** @brief Handle the GPIO state change event */
void AttnMonitor::handleGPIOEvent()
{
    gpiod_line_event gpioEvent;
    std::string logMessage;

    if (gpiod_line_event_read_fd(iv_gpioEventDescriptor.native_handle(),
                                 &gpioEvent) < 0)
    {
        logMessage = "GPIO line read failed";
        trace<level::INFO>(logMessage.c_str());
    }
    else
    {
        switch (gpiod_line_get_value(iv_gpioLine))
        {
            // active attention when gpio == 0
            case 0:
                logMessage = "Attention monitor calling attention handler";
                trace<level::INFO>(logMessage.c_str());
                attnHandler(iv_config);
                break;

            // gpio == 1, GPIO handler should not be executing
            case 1:
                logMessage = "GPIO handler out of sync";
                trace<level::INFO>(logMessage.c_str());
                break;

            // unexpected value
            default:
                logMessage = "GPIO line unexpected value";
                trace<level::INFO>(logMessage.c_str());
        }
    }
    scheduleGPIOEvent(); // continue monitoring gpio
}

/** @brief Request a GPIO line for monitoring attention events */
void AttnMonitor::requestGPIOEvent()
{
    if (0 != gpiod_line_request(iv_gpioLine, &iv_gpioConfig, 0))
    {
        std::string logMessage = "failed request for GPIO";
        trace<level::INFO>(logMessage.c_str());
    }
    else
    {
        int gpioLineFd;

        gpioLineFd = gpiod_line_event_get_fd(iv_gpioLine);
        if (gpioLineFd < 0)
        {
            std::string logMessage = "failed to get file descriptor";
            trace<level::INFO>(logMessage.c_str());
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
