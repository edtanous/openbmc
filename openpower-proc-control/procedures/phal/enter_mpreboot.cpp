/**
 * Copyright (C) 2020 IBM Corporation
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

#include "registration.hpp"

extern "C"
{
#include <libpdbg.h>
#include <libpdbg_sbe.h>
}

#include "extensions/phal/create_pel.hpp"
#include "extensions/phal/dump_utils.hpp"

#include <attributes_info.H>
#include <fmt/format.h>
#include <libphal.H>
#include <phal_exception.H>
#include <sys/wait.h>
#include <unistd.h>

#include <phosphor-logging/log.hpp>

#include <system_error>
#include <vector>

namespace openpower
{
namespace misc
{

/**
 * @brief Calls sbe_enter_mpipl on the SBE in the provided target.
 * @return void
 */
void sbeEnterMpReboot(struct pdbg_target* tgt)
{
    using namespace openpower::pel;
    using namespace openpower::phal;
    using namespace openpower::phal::sbe;
    using namespace openpower::phal::exception;
    using namespace phosphor::logging;

    try
    {
        mpiplEnter(tgt);
    }
    catch (const sbeError_t& sbeError)
    {
        log<level::ERR>(fmt::format("EnterMPIPL failed({}) on proc({})",
                                    sbeError.what(), pdbg_target_index(tgt))
                            .c_str());

        std::string event;
        bool dumpIsRequired = false;

        if (sbeError.errType() == SBE_CMD_TIMEOUT)
        {
            event = "org.open_power.Processor.Error.SbeChipOpTimeout";
            dumpIsRequired = true;
        }
        else
        {
            event = "org.open_power.Processor.Error.SbeChipOpFailure";
        }

        // SRC6 : [0:15] chip position [16:23] command class, [24:31] Type
        uint32_t index = pdbg_target_index(tgt);

        // TODO Replace these consts with pdbg defines once it is exported.
        // Ref : pdbg/libsbefifo/sbefifo_private.h
        constexpr auto SBEFIFO_CMD_CLASS_MPIPL = 0xA900;
        constexpr auto SBEFIFO_CMD_ENTER_MPIPL = 0x01;
        uint32_t cmd = SBEFIFO_CMD_CLASS_MPIPL | SBEFIFO_CMD_ENTER_MPIPL;

        // To store additional data about ffdc.
        FFDCData pelAdditionalData;
        pelAdditionalData.emplace_back("SRC6",
                                       std::to_string((index << 16) | cmd));
        auto logId = createSbeErrorPEL(event, sbeError, pelAdditionalData);

        if (dumpIsRequired)
        {
            // Request SBE Dump
            using namespace openpower::phal::dump;
            DumpParameters dumpParameters = {logId, index, SBE_DUMP_TIMEOUT,
                                             DumpType::SBE};
            requestDump(dumpParameters);
        }
        throw;
    }
    // Capture genaral libphal error
    catch (const phalError_t& phalError)
    {
        // Failure reported
        log<level::ERR>(fmt::format("captureFFDC: Exception({}) on proc({})",
                                    phalError.what(), pdbg_target_index(tgt))
                            .c_str());
        openpower::pel::createPEL(
            "org.open_power.Processor.Error.SbeChipOpFailure");
        throw;
    }

    log<level::INFO>(
        fmt::format("Enter MPIPL completed on proc({})", pdbg_target_index(tgt))
            .c_str());
}

/**
 * @brief initiate memory preserving reboot on each SBE.
 * @return void
 */
void enterMpReboot()
{
    using namespace phosphor::logging;
    struct pdbg_target* target;
    std::vector<pid_t> pidList;
    bool failed = false;
    pdbg_targets_init(NULL);
    ATTR_HWAS_STATE_Type hwasState;

    log<level::INFO>("Starting memory preserving reboot");
    pdbg_for_each_class_target("proc", target)
    {
        if (DT_GET_PROP(ATTR_HWAS_STATE, target, hwasState))
        {
            log<level::ERR>("Could not read HWAS_STATE attribute");
        }
        if (!hwasState.functional)
        {
            continue;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            log<level::ERR>("Fork failed while starting mp reboot");
            failed = true;
        }
        else if (pid == 0)
        {
            sbeEnterMpReboot(target);
            std::exit(EXIT_SUCCESS);
        }
        else
        {
            pidList.push_back(std::move(pid));
        }
    }

    for (auto& p : pidList)
    {
        int status = 0;
        waitpid(p, &status, 0);
        if (WEXITSTATUS(status))
        {
            log<level::ERR>("Memory preserving reboot failed");
            failed = true;
        }
    }

    if (failed)
    {
        std::exit(EXIT_FAILURE);
    }
}

REGISTER_PROCEDURE("enterMpReboot", enterMpReboot)

} // namespace misc
} // namespace openpower
