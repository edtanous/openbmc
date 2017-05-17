#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/manager.hpp>
#include "settings_manager.hpp"
#include "config.h"

int main(int argc, char *argv[])
{
    auto bus = sdbusplus::bus::new_default();

    // Add sdbusplus ObjectManager for the settings root.
    sdbusplus::server::manager::manager objManager(bus, SETTINGS_ROOT);

    phosphor::settings::Manager mgr(bus);

    bus.request_name(SETTINGS_BUSNAME);

    while(true)
    {
        bus.process_discard();
        bus.wait();
    }

    return 0;
}
