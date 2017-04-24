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

#include <endian.h>
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

std::unique_ptr<Target>& Targeting::getTarget(size_t pos)
{
    auto search = [pos](const auto& t)
    {
        return t->getPos() == pos;
    };

    auto target = find_if(targets.begin(), targets.end(), search);
    if (target == targets.end())
    {
        throw std::runtime_error("Target not found: " + std::to_string(pos));
    }
    else
    {
        return *target;
    }
}


static uint32_t noEndianSwap(uint32_t data)
{
    return data;
}

static uint32_t endianSwap(uint32_t data)
{
    return htobe32(data);
}

Targeting::Targeting(const std::string& fsiMasterDev,
                     const std::string& fsiSlaveDir) :
    fsiMasterPath(fsiMasterDev),
    fsiSlaveBasePath(fsiSlaveDir)
{
    swap_endian_t swapper = endianSwap;
    std::regex exp{"fsi1/slave@([0-9]{2}):00", std::regex::extended};

    if (!fs::exists(fsiMasterPath))
    {
        std::regex expOld{"hub@00/slave@([0-9]{2}):00", std::regex::extended};

        //Fall back to old (4.7) path
        exp = expOld;
        fsiMasterPath = fsiMasterDevPathOld;
        fsiSlaveBasePath = fsiSlaveBaseDirOld;

        //And don't swap the endianness of CFAM data
        swapper = noEndianSwap;
    }

    //Always create P0, the FSI master.
    targets.push_back(std::make_unique<Target>(0, fsiMasterPath, swapper));

    //Find the the remaining P9s dynamically based on which files show up
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
                                entry("DEVICE_NAME=%s", path.c_str()));
                continue;
            }

            path += "/raw";

            targets.push_back(std::make_unique<Target>(pos, path, swapper));
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
