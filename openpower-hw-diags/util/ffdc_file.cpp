#include "util/ffdc_file.hpp"

#include <errno.h>     // for errno
#include <fcntl.h>     // for open()
#include <string.h>    // for strerror()
#include <sys/stat.h>  // for open()
#include <sys/types.h> // for open()

#include <stdexcept>
#include <string>

namespace util
{

FFDCFile::FFDCFile(FFDCFormat format, uint8_t subType, uint8_t version) :
    format{format}, subType{subType}, version{version}
{
    // Open the temporary file for both reading and writing
    int fd = open(tempFile.getPath().c_str(), O_RDWR);
    if (fd == -1)
    {
        throw std::runtime_error{std::string{"Unable to open FFDC file: "} +
                                 strerror(errno)};
    }

    // Store file descriptor in FileDescriptor object
    descriptor.set(fd);
}

void FFDCFile::remove()
{
    // Close file descriptor.  Does nothing if descriptor was already closed.
    // Returns -1 if close failed.
    if (descriptor.close() == -1)
    {
        throw std::runtime_error{std::string{"Unable to close FFDC file: "} +
                                 strerror(errno)};
    }

    // Delete temporary file.  Does nothing if file was already deleted.
    // Throws an exception if the deletion failed.
    tempFile.remove();
}

} // namespace util
