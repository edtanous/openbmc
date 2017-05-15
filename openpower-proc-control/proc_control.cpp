/**
 * Copyright © 2017 IBM Corporation
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
#include <algorithm>
#include <functional>
#include <iostream>
#include <phosphor-logging/log.hpp>
#include <phosphor-logging/elog.hpp>
#include "registration.hpp"
#include "elog-errors.hpp"
#include <xyz/openbmc_project/Common/error.hpp>

using namespace openpower::util;

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
    catch (org::open_power::Proc::CFAM::SeekFailure& e)
    {
        commit<org::open_power::Proc::CFAM::SeekFailure>();
        return -1;
    }
    catch (org::open_power::Proc::CFAM::OpenFailure& e)
    {
        commit<org::open_power::Proc::CFAM::OpenFailure>();
        return -1;
    }
    catch (org::open_power::Proc::CFAM::WriteFailure& e)
    {
        commit<org::open_power::Proc::CFAM::WriteFailure>();
        return -1;
    }
    catch (org::open_power::Proc::CFAM::ReadFailure& e)
    {
        commit<org::open_power::Proc::CFAM::ReadFailure>();
        return -1;
    }
    catch (sdbusplus::xyz::openbmc_project::Common::Error::InvalidArgument& e)
    {
        commit<sdbusplus::xyz::openbmc_project::Common::Error::InvalidArgument>();
        return -1;
    }

    return 0;
}
