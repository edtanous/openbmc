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

#include "targeting.hpp"

#include <unistd.h>

#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>
#include <xyz/openbmc_project/Common/Device/error.hpp>
#include <xyz/openbmc_project/Common/File/error.hpp>

namespace openpower
{
namespace cfam
{
namespace access
{

constexpr auto cfamRegSize = 4;

using namespace openpower::targeting;
using namespace openpower::util;
namespace file_error = sdbusplus::xyz::openbmc_project::Common::File::Error;
namespace device_error = sdbusplus::xyz::openbmc_project::Common::Device::Error;

/**
 * Converts the CFAM register address used by the calling
 * code (because that's how it is in the spec) to the address
 * required by the device driver.
 */
static inline cfam_address_t makeOffset(cfam_address_t address)
{
    return (address & 0xFC00) | ((address & 0x03FF) << 2);
}

void writeReg(const std::unique_ptr<Target>& target, cfam_address_t address,
              cfam_data_t data)
{
    using namespace phosphor::logging;
    int rc = lseek(target->getCFAMFD(), makeOffset(address), SEEK_SET);
    if (rc < 0)
    {
        log<level::ERR>("Failed seeking on a processor CFAM",
                        entry("CFAM_ADDRESS=0x%X", address));

        using metadata = xyz::openbmc_project::Common::File::Seek;

        elog<file_error::Seek>(metadata::OFFSET(makeOffset(address)),
                               metadata::WHENCE(SEEK_SET),
                               metadata::ERRNO(errno),
                               metadata::PATH(target->getCFAMPath().c_str()));
    }

    data = target->swapEndian(data);

    rc = write(target->getCFAMFD(), &data, cfamRegSize);
    if (rc < 0)
    {
        using metadata = xyz::openbmc_project::Common::Device::WriteFailure;

        elog<device_error::WriteFailure>(
            metadata::CALLOUT_ERRNO(errno),
            metadata::CALLOUT_DEVICE_PATH(target->getCFAMPath().c_str()));
    }
}

cfam_data_t readReg(const std::unique_ptr<Target>& target,
                    cfam_address_t address)
{
    using namespace phosphor::logging;

    cfam_data_t data = 0;

    int rc = lseek(target->getCFAMFD(), makeOffset(address), SEEK_SET);
    if (rc < 0)
    {
        log<level::ERR>("Failed seeking on a processor CFAM",
                        entry("CFAM_ADDRESS=0x%X", address));

        using metadata = xyz::openbmc_project::Common::File::Seek;

        elog<file_error::Seek>(metadata::OFFSET(makeOffset(address)),
                               metadata::WHENCE(SEEK_SET),
                               metadata::ERRNO(errno),
                               metadata::PATH(target->getCFAMPath().c_str()));
    }

    rc = read(target->getCFAMFD(), &data, cfamRegSize);
    if (rc < 0)
    {
        using metadata = xyz::openbmc_project::Common::Device::ReadFailure;

        elog<device_error::ReadFailure>(
            metadata::CALLOUT_ERRNO(errno),
            metadata::CALLOUT_DEVICE_PATH(target->getCFAMPath().c_str()));
    }

    return target->swapEndian(data);
}

void writeRegWithMask(const std::unique_ptr<Target>& target,
                      cfam_address_t address, cfam_data_t data,
                      cfam_mask_t mask)
{
    cfam_data_t readData = readReg(target, address);

    readData &= ~mask;
    readData |= (data & mask);

    writeReg(target, address, readData);
}

} // namespace access
} // namespace cfam
} // namespace openpower
