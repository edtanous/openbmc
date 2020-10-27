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
#include "filedescriptor.hpp"

#include <unistd.h>

#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>
#include <xyz/openbmc_project/Common/File/error.hpp>

#include <stdexcept>

namespace openpower
{
namespace util
{

namespace file_error = sdbusplus::xyz::openbmc_project::Common::File::Error;

FileDescriptor::FileDescriptor(const std::string& path)
{
    using namespace phosphor::logging;

    fd = open(path.c_str(), O_RDWR | O_SYNC);

    if (fd < 0)
    {
        using metadata = xyz::openbmc_project::Common::File::Open;

        elog<file_error::Open>(metadata::ERRNO(errno),
                               metadata::PATH(path.c_str()));
    }
}

FileDescriptor::~FileDescriptor()
{
    if (fd >= 0)
    {
        close(fd);
    }
}

} // namespace util
} // namespace openpower
