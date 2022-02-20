#pragma once

#include "sys.hpp"
#include "sys_file.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <optional>
#include <string>

namespace binstore
{

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
    explicit SysFileImpl(const std::string& path,
                         std::optional<size_t> offset = std::nullopt,
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
