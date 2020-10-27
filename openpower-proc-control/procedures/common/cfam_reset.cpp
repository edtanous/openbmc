#include <unistd.h>

#include <gpiod.hpp>
#include <phosphor-logging/log.hpp>
#include <registration.hpp>

#include <chrono>
#include <fstream>
#include <system_error>
#include <thread>

namespace openpower
{
namespace misc
{

constexpr auto cfamResetPath = "/sys/class/fsi-master/fsi0/device/cfam_reset";

using namespace phosphor::logging;

/**
 * @brief Reset the CFAM using the appropriate GPIO
 * @return void
 */
void cfamReset()
{

    // First look if system supports kernel sysfs based cfam reset
    // If it does then write a 1 and let the kernel handle the reset
    std::ofstream file;
    file.open(cfamResetPath);
    if (!file)
    {
        log<level::DEBUG>("system does not support kernel cfam reset, default "
                          "to using libgpiod");
    }
    else
    {
        // Write a 1 to have kernel toggle the reset
        file << "1";
        file.close();
        log<level::DEBUG>("cfam reset via sysfs complete");
        return;
    }

    // No kernel support so toggle gpio from userspace
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

REGISTER_PROCEDURE("cfamReset", cfamReset)

} // namespace misc
} // namespace openpower
