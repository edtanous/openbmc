/**
 * Copyright © 2019 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nmi_interface.hpp"

#include <sdbusplus/bus.hpp>

int main(int argc, char* argv[])
{

    constexpr auto BUSPATH_NMI = "/xyz/openbmc_project/control/host0/nmi";
    constexpr auto BUSNAME_NMI = "xyz.openbmc_project.Control.Host.NMI";
    auto bus = sdbusplus::bus::new_default();

    // Add sdbusplus ObjectManager
    sdbusplus::server::manager::manager objManager(bus, BUSPATH_NMI);
    openpower::proc::NMI NMI(bus, BUSPATH_NMI);
    bus.request_name(BUSNAME_NMI);

    while (true)
    {
        bus.process_discard();
        bus.wait();
    }

    return 0;
}
