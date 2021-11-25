#include "temporary_file.hpp"

#include <fmt/format.h>
#include <unistd.h>

#include <phosphor-logging/elog-errors.hpp>

#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace openpower::util
{
using namespace phosphor::logging;

TemporaryFile::TemporaryFile()
{
    // Build template path required by mkstemp()
    std::string templatePath =
        fs::temp_directory_path() / "openpower-proc-control-XXXXXX";

    // Generate unique file name, create file, and open it.  The XXXXXX
    // characters are replaced by mkstemp() to make the file name unique.
    int fd = mkstemp(templatePath.data());
    if (fd == -1)
    {
        throw std::runtime_error{
            std::string{"Unable to create temporary file: "} + strerror(errno)};
    }

    // Store path to temporary file
    path = templatePath;

    // Close file descriptor
    if (close(fd) == -1)
    {
        // Save errno value; will likely change when we delete temporary file
        int savedErrno = errno;

        // Delete temporary file.  The destructor won't be called because
        // the exception below causes this constructor to exit without
        // completing.
        remove();

        throw std::runtime_error{
            std::string{"Unable to close temporary file: "} +
            strerror(savedErrno)};
    }
}

void TemporaryFile::remove()
{
    if (!path.empty())
    {
        // Delete temporary file from file system
        fs::remove(path);

        // Clear path to indicate file has been deleted
        path.clear();
    }
}

} // namespace openpower::util
