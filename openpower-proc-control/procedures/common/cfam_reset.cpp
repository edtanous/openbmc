#include <unistd.h>

#include <chrono>
#include <gpiod.hpp>
#include <phosphor-logging/log.hpp>
#include <registration.hpp>
#include <system_error>
#include <thread>

namespace openpower
{
namespace misc
{

using namespace phosphor::logging;

/**
 * @brief Reset the CFAM using the appropriate GPIO
 * @return void
 */
void cfamReset()
{
    const std::string cfamReset = {"cfam-reset"};
    auto line = gpiod::find_line(cfamReset);
    if (!line)
    {
        log<level::ERR>("failed to find cfam-reset line");
        throw std::system_error(ENODEV, std::system_category());
    }

    // Configure this app to own the gpio while doing the reset
    gpiod::line_request conf;
    conf.consumer = "cfamReset";
    conf.request_type = gpiod::line_request::DIRECTION_OUTPUT;
    line.request(conf);

    // Put chips into reset
    line.set_value(0);

    // Sleep one second to ensure reset processed
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    // Take chips out of reset
    line.set_value(1);
}

REGISTER_PROCEDURE("cfamReset", cfamReset);

} // namespace misc
} // namespace openpower
