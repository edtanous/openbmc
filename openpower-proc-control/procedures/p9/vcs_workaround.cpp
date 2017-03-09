/**
 * Copyright © 2017 IBM Corporation
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
#include <phosphor-logging/log.hpp>
#include "cfam_access.hpp"
#include "p9_cfam.hpp"
#include "targeting.hpp"

namespace openpower
{
namespace p9
{

using namespace phosphor::logging;
using namespace openpower::cfam::access;
using namespace openpower::cfam::p9;
using namespace openpower::targeting;


/**
 * @brief Performs the 'VCS Workaround' on all P9s in the system.
 * @return void
 */
void vcsWorkaround()
{
    Targeting targets;
    const auto& master = *(targets.begin());

    log<level::INFO>("Running P9 procedure vcsWorkaround",
                     entry("NUM_PROCS=%d", targets.size()));

    //Set asynchronous clock mode
    writeReg(master, P9_LL_MODE_REG, 0x00000001);

    for (const auto& t : targets)
    {
        //Unfence PLL controls
        writeRegWithMask(t, P9_ROOT_CTRL0,
                         0x00000000, 0x00010000);

        //Assert Perv chiplet endpoint reset
        writeRegWithMask(t, P9_PERV_CTRL0,
                         0x40000000, 0x40000000);

        //Enable Nest PLL
        writeRegWithMask(t, P9_PERV_CTRL0,
                         0x00000001, 0x00000001);
    }
}

}
}
