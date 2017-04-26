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
#include <stdexcept>
#include <unistd.h>
#include "filedescriptor.hpp"
#include <phosphor-logging/elog.hpp>
#include "elog-errors.hpp"

namespace openpower
{
namespace util
{

FileDescriptor::FileDescriptor(const std::string& path)
{
    using namespace phosphor::logging;

    fd = open(path.c_str(), O_RDWR | O_SYNC);

    if (fd < 0)
    {
        elog<org::open_power::Proc::CFAM::OpenFailure>(
            org::open_power::Proc::CFAM::OpenFailure::ERRNO(errno),
            org::open_power::Proc::CFAM::OpenFailure::PATH(path.c_str()));
    }
}


FileDescriptor::~FileDescriptor()
{
    if (fd >= 0)
    {
        close(fd);
    }
}

}
}
