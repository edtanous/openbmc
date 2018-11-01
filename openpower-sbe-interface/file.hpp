#pragma once

#include <unistd.h>

#include <sstream>

namespace openpower
{
namespace sbe
{
namespace internal
{

/** @class FileDescriptor
 *  @brief Provide RAII file descriptor
 */
class FileDescriptor
{
  private:
    /** @brief File descriptor for the SBE FIFO device */
    int fd = -1;

  public:
    FileDescriptor() = delete;
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;
    FileDescriptor(FileDescriptor&&) = delete;
    FileDescriptor& operator=(FileDescriptor&&) = delete;

    /** @brief Opens the input file and saves the file descriptor
     *
     *  @param[in] devPath - Path of the file
     *  @para,[in] accessModes - File access modes
     */
    FileDescriptor(const char* devPath, int accessModes)
    {
        fd = open(devPath, accessModes);
        if (fd < 0)
        {
            // TODO:use elog infrastructure
            std::ostringstream errMsg;
            errMsg << "Opening the device with device path:" << devPath
                   << " and access modes:" << accessModes
                   << ",Failed with errno" << errno;
            throw std::runtime_error(errMsg.str().c_str());
        }
    }

    /** @brief Saves File descriptor and uses it to do file operation
     *
     *  @param[in] fd - File descriptor
     */
    FileDescriptor(int fd) : fd(fd)
    {
        // Nothing
    }

    ~FileDescriptor()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    int operator()()
    {
        return fd;
    }
};

} // namespace internal
} // namespace sbe
} // namespace openpower
