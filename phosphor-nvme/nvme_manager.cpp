#include "nvme_manager.hpp"

#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>

#define MONITOR_INTERVAL_SECONDS 1
namespace phosphor
{
namespace nvme
{

using namespace std;
using namespace phosphor::logging;

void Nvme::run()
{
    init();

    std::function<void()> callback(std::bind(&Nvme::read, this));
    try
    {
        u_int64_t interval = MONITOR_INTERVAL_SECONDS * 1000000;
        _timer.restart(std::chrono::microseconds(interval));
    }
    catch (const std::exception& e)
    {
        log<level::ERR>("Error in polling loop. "),
            entry("ERROR = %s", e.what());
    }
}

void Nvme::init()
{
}

void Nvme::read()
{
}
} // namespace nvme
} // namespace phosphor
