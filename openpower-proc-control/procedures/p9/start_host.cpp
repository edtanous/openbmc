/**
 * Copyright (C) 2017 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "cfam_access.hpp"
#include "ext_interface.hpp"
#include "p9_cfam.hpp"
#include "registration.hpp"
#include "targeting.hpp"

#include <phosphor-logging/log.hpp>

namespace openpower
{
namespace p9
{

using namespace phosphor::logging;
using namespace openpower::cfam::access;
using namespace openpower::cfam::p9;
using namespace openpower::targeting;

/**
 * @brief Starts the self boot engine on P9 position 0 to kick off a boot.
 * @return void
 */
void startHost()
{
    Targeting targets;
    const auto& master = *(targets.begin());

    log<level::INFO>("Running P9 procedure startHost",
                     entry("NUM_PROCS=%d", targets.size()));

    // Ensure asynchronous clock mode is set
    writeReg(master, P9_LL_MODE_REG, 0x00000001);

    // Clock mux select override
    for (const auto& t : targets)
    {
        writeRegWithMask(t, P9_ROOT_CTRL8, 0x0000000C, 0x0000000C);
    }

    // Enable P9 checkstop to be reported to the BMC

    // Setup FSI2PIB to report checkstop
    writeReg(master, P9_FSI_A_SI1S, 0x20000000);

    // Enable Xstop/ATTN interrupt
    writeReg(master, P9_FSI2PIB_TRUE_MASK, 0x60000000);

    // Arm it
    writeReg(master, P9_FSI2PIB_INTERRUPT, 0xFFFFFFFF);

    // Kick off the SBE to start the boot

    // Choose seeprom side to boot from
    cfam_data_t sbeSide = 0;
    if (getBootCount() > 0)
    {
        sbeSide = 0;
        log<level::INFO>("Setting SBE seeprom side to 0",
                         entry("SBE_SIDE_SELECT=%d", 0));
    }
    else
    {
        sbeSide = 0x00004000;
        log<level::INFO>("Setting SBE seeprom side to 1",
                         entry("SBE_SIDE_SELECT=%d", 1));
    }
    // Bit 17 of the ctrl status reg indicates sbe seeprom boot side
    // 0 -> Side 0, 1 -> Side 1
    writeRegWithMask(master, P9_SBE_CTRL_STATUS, sbeSide, 0x00004000);

    // Ensure SBE start bit is 0 to handle warm reboot scenarios
    writeRegWithMask(master, P9_CBS_CS, 0x00000000, 0x80000000);

    // Start the SBE
    writeRegWithMask(master, P9_CBS_CS, 0x80000000, 0x80000000);
}

REGISTER_PROCEDURE("startHost", startHost);

} // namespace p9
} // namespace openpower
