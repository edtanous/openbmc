#include "fw_update_watch.hpp"

#include <fmt/format.h>

#include <phosphor-logging/elog-errors.hpp>
#include <sdeventplus/event.hpp>

int main()
{
    auto eventRet = 0;

    try
    {
        auto bus = sdbusplus::bus::new_default();

        auto event = sdeventplus::Event::get_default();

        // create watch for interface added in software update.
        openpower::phal::fwupdate::Watch eWatch(bus);

        bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);

        // Watch for software update
        eventRet = event.loop();
    }
    catch (const std::exception& e)
    {
        using namespace phosphor::logging;
        log<level::ERR>(
            fmt::format("Exception reported: [{}]", e.what()).c_str());
    }

    return eventRet;
}
