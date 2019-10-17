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

#include <algorithm>
#include <functional>
#include <iostream>
#include <org/open_power/Proc/FSI/error.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/log.hpp>
#include <xyz/openbmc_project/Common/Device/error.hpp>
#include <xyz/openbmc_project/Common/File/error.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

using namespace openpower::util;

namespace common_error = sdbusplus::xyz::openbmc_project::Common::Error;
namespace device_error = sdbusplus::xyz::openbmc_project::Common::Device::Error;
namespace file_error = sdbusplus::xyz::openbmc_project::Common::File::Error;
namespace fsi_error = sdbusplus::org::open_power::Proc::FSI::Error;

void usage(char** argv, const ProcedureMap& procedures)
{
    std::cerr << "Usage: " << argv[0] << " [action]\n";
    std::cerr << "   actions:\n";

    for (const auto& p : procedures)
    {
        std::cerr << "     " << p.first << "\n";
    }
}

int main(int argc, char** argv)
{
    using namespace phosphor::logging;
    const ProcedureMap& procedures = Registration::getProcedures();

    if (argc != 2)
    {
        usage(argv, procedures);
        return -1;
    }

    std::string action{argv[1]};

    auto procedure = procedures.find(action);

    if (procedure == procedures.end())
    {
        usage(argv, procedures);
        return -1;
    }

    try
    {
        procedure->second();
    }
    catch (file_error::Seek& e)
    {
        commit<file_error::Seek>();
        return -1;
    }
    catch (file_error::Open& e)
    {
        commit<file_error::Open>();
        return -1;
    }
    catch (device_error::WriteFailure& e)
    {
        commit<device_error::WriteFailure>();
        return -1;
    }
    catch (device_error::ReadFailure& e)
    {
        commit<device_error::ReadFailure>();
        return -1;
    }
    catch (common_error::InvalidArgument& e)
    {
        commit<common_error::InvalidArgument>();
        return -1;
    }
    catch (fsi_error::MasterDetectionFailure& e)
    {
        commit<fsi_error::MasterDetectionFailure>();
        return -1;
    }
    catch (fsi_error::SlaveDetectionFailure& e)
    {
        commit<fsi_error::SlaveDetectionFailure>();
        return -1;
    }
    // TODO ibm-openbmc#1470
    catch (common_error::InternalFailure& e)
    {
        commit<common_error::InternalFailure>();
        return -1;
    }

    return 0;
}
