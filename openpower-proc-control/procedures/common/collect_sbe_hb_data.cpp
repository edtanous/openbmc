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
#include "p9_cfam.hpp"
#include "registration.hpp"
#include "targeting.hpp"

#include <phosphor-logging/log.hpp>

namespace openpower
{
namespace p9
{
namespace debug
{
// SBE messaging register - cfam 2809
union sbeMsgReg_t
{
    uint32_t data32;

    struct
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint32_t reserved2 : 6;
        uint32_t minorStep : 6;
        uint32_t majorStep : 8;
        uint32_t currState : 4;
        uint32_t prevState : 4;
        uint32_t reserved1 : 2;
        uint32_t asyncFFDC : 1;
        uint32_t sbeBooted : 1;
#else
        uint32_t sbeBooted : 1;
        uint32_t asyncFFDC : 1;
        uint32_t reserved1 : 2;
        uint32_t prevState : 4;
        uint32_t currState : 4;
        uint32_t majorStep : 8;
        uint32_t minorStep : 6;
        uint32_t reserved2 : 6;
#endif
    } PACKED;
};

// HB mailbox scratch register 5 - cfam 283C
union MboxScratch5_HB_t
{
    uint32_t data32;
    struct
    {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint32_t minorStep : 8;    // 24:31
        uint32_t majorStep : 8;    // 16:23
        uint32_t internalStep : 4; // 12:15
        uint32_t reserved : 2;     // 10:11
        uint32_t stepFinish : 1;   // 9
        uint32_t stepStart : 1;    // 8
        uint32_t magic : 8;        // 0:7
#else
        uint32_t magic : 8;        // 0:7
        uint32_t stepStart : 1;    // 8
        uint32_t stepFinish : 1;   // 9
        uint32_t reserved : 2;     // 10:11
        uint32_t internalStep : 4; // 12:15
        uint32_t majorStep : 8;    // 16:23
        uint32_t minorStep : 8;    // 24:31
#endif
    } PACKED;
};

static constexpr uint8_t HB_MBX5_VALID_FLAG = 0xAA;

/**
 * @brief Capture SBE and HB istep information on watchdog timeout
 * @return void
 */
void collectSBEHBData()
{
    using namespace openpower::targeting;
    using namespace openpower::cfam::p9;
    using namespace openpower::cfam::access;
    using namespace phosphor::logging;

    Targeting targets;

    for (const auto& proc : targets)
    {
        // Read and parse SBE messaging register
        try
        {
            auto readData = readReg(proc, P9_SBE_MSG_REGISTER);
            auto msg = reinterpret_cast<const sbeMsgReg_t*>(&readData);
            log<level::INFO>("SBE status register",
                             entry("PROC=%d", proc->getPos()),
                             entry("SBE_MAJOR_ISTEP=%d", msg->PACKED.majorStep),
                             entry("SBE_MINOR_ISTEP=%d", msg->PACKED.minorStep),
                             entry("REG_VAL=0x%08X", msg->data32));
        }
        catch (const std::exception& e)
        {
            log<level::ERR>(e.what());
            // We want to continue - capturing as much info as possible
        }
    }

    const auto& master = *(targets.begin());
    // Read and parse HB messaging register
    try
    {
        auto readData = readReg(master, P9_HB_MBX5_REG);
        auto msg = reinterpret_cast<const MboxScratch5_HB_t*>(&readData);
        if (HB_MBX5_VALID_FLAG == msg->PACKED.magic)
        {
            log<level::INFO>("HB MBOX 5 register",
                             entry("HB_MAJOR_ISTEP=%d", msg->PACKED.majorStep),
                             entry("HB_MINOR_ISTEP=%d", msg->PACKED.minorStep),
                             entry("REG_VAL=0x%08X", msg->data32));
        }
    }
    catch (const std::exception& e)
    {
        log<level::ERR>(e.what());
        // We want to continue - capturing as much info as possible
    }
}

REGISTER_PROCEDURE("collectSBEHBData", collectSBEHBData);

} // namespace debug
} // namespace p9
} // namespace openpower
