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

namespace openpower
{
namespace p9
{

using namespace openpower::cfam::access;
using namespace openpower::cfam::p9;
using namespace openpower::targeting;

/**
 * @Brief Sets the P9 FSI clock to synchronous mode.
 */
void setSynchronousFSIClock()
{
    Targeting targets;
    const auto& master = *(targets.begin());

    // Set bit 31 to 0
    writeRegWithMask(master, P9_LL_MODE_REG, 0x00000000, 0x00000001);
}

REGISTER_PROCEDURE("setSyncFSIClock", setSynchronousFSIClock)

} // namespace p9
} // namespace openpower
