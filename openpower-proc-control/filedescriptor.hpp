#pragma once

#include <fcntl.h>
#include <string>

namespace openpower
{
namespace util
{

/**
 * Class to wrap a file descriptor.
 *
 * Opens the descriptor in the constructor, and
 * then closes it when destroyed.
 */
class FileDescriptor
{
    public:

        FileDescriptor() = delete;
        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor(FileDescriptor&&) = default;
        FileDescriptor& operator=(const FileDescriptor) = delete;
        FileDescriptor& operator=(FileDescriptor&&) = default;

        /**
         * Creates a file descriptor by opening the device
         * path passed in.
         *
         * @param path[in] - the device path that will be open
         */
        FileDescriptor(const std::string& path);

        /**
         * Closes the file.
         */
        ~FileDescriptor();

        /**
         * The method to access the file descriptor value
         */
        inline auto get() const
        {
            return fd;
        }

    private:

        /**
         * The actual file descriptor
         */
        int fd;
};

}
}
