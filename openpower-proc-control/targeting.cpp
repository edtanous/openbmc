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
#include <experimental/filesystem>
#include <phosphor-logging/log.hpp>
#include <regex>
#include "targeting.hpp"

namespace openpower
{
namespace targeting
{

using namespace phosphor::logging;
namespace fs = std::experimental::filesystem;

int Target::getCFAMFD()
{
    if (cfamFD.get() == nullptr)
    {
        cfamFD = std::make_unique<
            openpower::util::FileDescriptor>(getCFAMPath());
    }

    return cfamFD->get();
}


Targeting::Targeting(const std::string& fsiMasterDev,
                     const std::string& fsiSlaveDir) :
    fsiMasterPath(fsiMasterDev),
    fsiSlaveBasePath(fsiSlaveDir)
{
    //Always create P0, the FSI master.
    targets.push_back(std::make_unique<Target>(0, fsiMasterPath));

    //Find the the remaining P9s dynamically based on which files show up
    std::regex exp{"slave@([0-9]{2}):00", std::regex::extended};

    for (auto& file : fs::directory_iterator(fsiSlaveBasePath))
    {
        std::smatch match;
        std::string path = file.path();
        if (std::regex_search(path, match, exp))
        {
            auto pos = atoi(match[1].str().c_str());
            if (pos == 0)
            {
                log<level::ERR>("Unexpected FSI slave device name found",
                                entry("DEVICE_NAME=%d", path.c_str()));
                continue;
            }

            path += "/raw";

            targets.push_back(std::make_unique<Target>(pos, path));
        }
    }

    auto sortTargets = [](const std::unique_ptr<Target>& left,
                          const std::unique_ptr<Target>& right)
    {
        return left->getPos() < right->getPos();
    };
    std::sort(targets.begin(), targets.end(), sortTargets);
}

}
}
