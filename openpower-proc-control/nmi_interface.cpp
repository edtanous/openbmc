/**
 * Copyright (C) 2019 IBM Corporation
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

#include "nmi_interface.hpp"

#include <libpdbg.h>

#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

namespace openpower
{
namespace proc
{

NMI::NMI(sdbusplus::bus::bus& bus, const char* path) :
    Interface(bus, path), bus(bus), objectPath(path)
{}

void NMI::nMI()
{
    using namespace phosphor::logging;
    using InternalFailure =
        sdbusplus::xyz::openbmc_project::Common::Error::InternalFailure;

    struct pdbg_target* target;

    pdbg_for_each_class_target("thread", target)
    {
        if (pdbg_target_probe(target) != PDBG_TARGET_ENABLED)
            continue;

        if (thread_stop(target) < 0 || !thread_status(target).quiesced)
        {
            log<level::ERR>("Failed to stop all threads");
            report<InternalFailure>();
            return;
        }
    }

    if (thread_sreset_all() < 0)
    {
        log<level::ERR>("Failed to sreset all threads");
        report<InternalFailure>();
    }
}
} // namespace proc
} // namespace openpower
