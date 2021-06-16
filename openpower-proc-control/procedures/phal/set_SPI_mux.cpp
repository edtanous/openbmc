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
#include "cfam_access.hpp"
#include "p10_cfam.hpp"
#include "registration.hpp"
#include "targeting.hpp"

namespace openpower
{
namespace p10
{

using namespace openpower::cfam::access;
using namespace openpower::cfam::p10;
using namespace openpower::targeting;

/**
 * Sets the mux on the P10 to the FSI SPI masters rather than the PIB SPI
 * masters. This should only be executed before the host is powering on since
 * the host will set the mux to the PIB SPI masters.
 */
void setSPIMux()
{
    Targeting targets;

    for (const auto& t : targets)
    {
        writeRegWithMask(t, P10_ROOT_CTRL8, 0xF0000000, 0xF0000000);
    }
}

REGISTER_PROCEDURE("setSPIMux", setSPIMux)

} // namespace p10
} // namespace openpower
