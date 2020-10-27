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

#include <libpdbg.h>
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
    using namespace phosphor::logging;
    int error = 0;
    if ((error = sbe_mpipl_enter(tgt)) < 0)
    {
        log<level::ERR>("Failed to initiate memory preserving reboot");
        // TODO Create a PEL in the future for this failure case.
        throw std::system_error(error, std::generic_category(),
                                "Failed to initiate memory preserving reboot");
    }
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

    log<level::INFO>("Starting memory preserving reboot");
    pdbg_for_each_class_target("pib", target)
    {
        if (pdbg_target_probe(target) != PDBG_TARGET_ENABLED)
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
