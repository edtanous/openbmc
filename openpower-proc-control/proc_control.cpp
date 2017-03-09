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
#include "registration.hpp"

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
    catch (std::exception& e)
    {
        //TODO: commit an actual error that does a callout
        phosphor::logging::log<phosphor::logging::level::ERR>(e.what());
        return -1;
    }

    return 0;
}
