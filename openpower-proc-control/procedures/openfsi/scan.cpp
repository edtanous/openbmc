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
#include "registration.hpp"

#include <org/open_power/Proc/FSI/error.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>

#include <filesystem>
#include <fstream>

namespace openpower
{
namespace openfsi
{

using namespace phosphor::logging;
namespace fsi_error = sdbusplus::org::open_power::Proc::FSI::Error;

constexpr auto masterScanPath = "/sys/class/fsi-master/fsi0/rescan";
constexpr auto hubScanPath = "/sys/class/fsi-master/fsi1/rescan";
constexpr auto masterCalloutPath = "/sys/class/fsi-master/fsi0/slave@00:00/raw";

/**
 * Writes a 1 to the sysfs file passed in to trigger
 * the device driver to do an FSI scan.
 *
 * @param[in] path - the sysfs path to write a 1 to
 */
static void doScan(const std::string& path)
{
    std::ofstream file;

    file.exceptions(std::ofstream::failbit | // logic error on operation
                    std::ofstream::badbit |  // read/write error on operation
                    std::ofstream::eofbit);  // end of file reached
    try
    {
        file.open(path);
        file << "1";
    }
    catch (std::exception& e)
    {
        auto err = errno;
        throw std::system_error(err, std::generic_category());
    }
}

/**
 * Performs an FSI master scan followed by an FSI hub scan.
 * This is where the device driver detects which chips are present.
 *
 * This is unrelated to scanning a ring out of a chip.
 */
void scan()
{
    // Note: Currently the FSI device driver will always return success on both
    // the master and hub scans.  The only way we can detect something
    // went wrong is if the master scan didn't create the hub scan file, so
    // we will check for that.
    // It is possible the driver will be updated in the future to actually
    // return a failure so the code will still check for them.

    try
    {
        doScan(masterScanPath);
    }
    catch (std::system_error& e)
    {
        log<level::ERR>("Failed to run the FSI master scan");

        using metadata = org::open_power::Proc::FSI::MasterDetectionFailure;

        elog<fsi_error::MasterDetectionFailure>(
            metadata::CALLOUT_ERRNO(e.code().value()),
            metadata::CALLOUT_DEVICE_PATH(masterCalloutPath));
    }

    if (!std::filesystem::exists(hubScanPath))
    {
        log<level::ERR>("The FSI master scan did not create a hub scan file");

        using metadata = org::open_power::Proc::FSI::MasterDetectionFailure;

        elog<fsi_error::MasterDetectionFailure>(
            metadata::CALLOUT_ERRNO(0),
            metadata::CALLOUT_DEVICE_PATH(masterCalloutPath));
    }

    try
    {
        doScan(hubScanPath);
    }
    catch (std::system_error& e)
    {
        // If the device driver is ever updated in the future to fail the sysfs
        // write call on a scan failure then it should also provide some hints
        // about which hardware failed so we can do an appropriate callout
        // here.  At this point in time, the driver shouldn't ever fail so
        // we won't worry about guessing at the callout.

        log<level::ERR>("Failed to run the FSI hub scan");

        using metadata = org::open_power::Proc::FSI::SlaveDetectionFailure;

        elog<fsi_error::SlaveDetectionFailure>(
            metadata::ERRNO(e.code().value()));
    }
}

REGISTER_PROCEDURE("scanFSI", scan)

} // namespace openfsi
} // namespace openpower
