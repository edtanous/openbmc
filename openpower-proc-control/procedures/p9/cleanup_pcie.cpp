/**
 * Copyright (C) 2018 IBM Corporation
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

using namespace phosphor::logging;
using namespace openpower::cfam::access;
using namespace openpower::cfam::p9;
using namespace openpower::targeting;

/**
 * @brief Disables PCIE drivers and receiver in the PCIE root ctrl 1 register
 * @return void
 */
void cleanupPcie()
{
    Targeting targets;

    log<level::INFO>("Running P9 procedure cleanupPcie");

    // Disable the PCIE drivers and receiver on all CPUs
    for (const auto& target : targets)
    {
        writeReg(target, P9_ROOT_CTRL1_CLEAR, 0x00001C00);
    }
}

REGISTER_PROCEDURE("cleanupPcie", cleanupPcie);

} // namespace p9
} // namespace openpower
