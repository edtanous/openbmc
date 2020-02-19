#include "attn_monitor.hpp"

#include "attn_handler.hpp"

#include <phosphor-logging/log.hpp>

using namespace phosphor::logging;

namespace attn
{

/** @brief Register a callback for gpio event */
void AttnMonitor::scheduleGPIOEvent()
{
    std::string logMessage = "[ATTN] ... waiting for events ...";
    log<level::INFO>(logMessage.c_str());

    // Register async callback, note that callback is a
    // lambda function with "this" pointer captured
    iv_gpioEventDescriptor.async_wait(
        boost::asio::posix::stream_descriptor::wait_read,
        [this](const boost::system::error_code& ec) {
            if (ec)
            {
                std::string logMessage = "[ATTN] ATTN GPIO Async error: " +
                                         std::string(ec.message());
                log<level::INFO>(logMessage.c_str());
            }
            else
            {
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
        logMessage = "[ATTN] ATTN GPIO Failed can't read file descriptor!";
        log<level::INFO>(logMessage.c_str());
    }
    else
    {
        switch (gpiod_line_get_value(iv_gpioLine))
        {
            // active attention when gpio == 0
            case 0:
                attnHandler(iv_breakpoints);
                break;

            // gpio == 1, GPIO handler should not be executing
            case 1:
                logMessage = "[ATTN] ATTN GPIO sync!";
                log<level::INFO>(logMessage.c_str());
                break;

            // unexpected value
            default:
                logMessage = "[ATTN] ATTN GPIO read unexpected valuel!";
                log<level::INFO>(logMessage.c_str());
        }
    }
    scheduleGPIOEvent(); // continue monitoring gpio
}

/** @brief Request a GPIO line for monitoring attention events */
void AttnMonitor::requestGPIOEvent()
{
    if (0 != gpiod_line_request(iv_gpioLine, &iv_gpioConfig, 0))
    {
        std::string logMessage = "[ATTN] failed request for GPIO";
        log<level::INFO>(logMessage.c_str());
    }
    else
    {
        int gpioLineFd;

        gpioLineFd = gpiod_line_event_get_fd(iv_gpioLine);
        if (gpioLineFd < 0)
        {
            std::string logMessage = "[ATTN] failed to get file descriptor";
            log<level::INFO>(logMessage.c_str());
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
