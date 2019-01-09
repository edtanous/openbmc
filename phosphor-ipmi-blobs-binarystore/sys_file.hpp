#pragma once

#include "sys.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <string>

namespace binstore
{

/**
 * @brief Represents a file that supports read/write semantics
 * TODO: leverage stdplus's support for smart file descriptors when it's ready.
 */
class SysFile
{
  public:
    virtual ~SysFile() = default;

    /**
     * @brief Reads content at pos to char* buffer
     * @param pos The byte pos into the file to read from
     * @param count How many bytes to read
     * @param buf Output data
     * @returns The size of data read
     * @throws std::system_error if read operation cannot be completed
     */
    virtual size_t readToBuf(size_t pos, size_t count, char* buf) const = 0;

    /**
     * @brief Reads content at pos
     * @param pos The byte pos into the file to read from
     * @param count How many bytes to read
     * @returns The data read in a vector, whose size might be smaller than
     *          count if there is not enough to read.
     * @throws std::system_error if read operation cannot be completed
     */
    virtual std::string readAsStr(size_t pos, size_t count) const = 0;

    /**
     * @brief Reads all the content in file after pos
     * @param pos The byte pos to read from
     * @returns The data read in a vector, whose size might be smaller than
     *          count if there is not enough to read.
     * @throws std::system_error if read operation cannot be completed
     */
    virtual std::string readRemainingAsStr(size_t pos) const = 0;

    /**
     * @brief Writes all of data into file at pos
     * @param pos The byte pos to write
     * @returns void
     * @throws std::system_error if write operation cannot be completed or
     *         not all of the bytes can be written
     */
    virtual void writeStr(const std::string& data, size_t pos) = 0;
};

class SysFileImpl : public SysFile
{
  public:
    /**
     * @brief Constructs sysFile specified by path and offset
     * @param path The file path
     * @param offset The byte offset relatively. Reading a sysfile at position 0
     *     actually reads underlying file at 'offset'
     * @param sys Syscall operation interface
     */
    explicit SysFileImpl(const std::string& path, size_t offset = 0,
                         const internal::Sys* sys = &internal::sys_impl);
    ~SysFileImpl();
    SysFileImpl() = delete;
    SysFileImpl(const SysFileImpl&) = delete;
    SysFileImpl& operator=(SysFileImpl) = delete;

    size_t readToBuf(size_t pos, size_t count, char* buf) const override;
    std::string readAsStr(size_t pos, size_t count) const override;
    std::string readRemainingAsStr(size_t pos) const override;
    void writeStr(const std::string& data, size_t pos) override;

  private:
    int fd_;
    size_t offset_;
    void lseek(size_t pos) const;
    const internal::Sys* sys;
};

} // namespace binstore
