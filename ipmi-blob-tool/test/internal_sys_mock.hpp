#pragma once

#include <unistd.h>

#include <ipmiblob/internal/sys_interface.hpp>

#include <gmock/gmock.h>

namespace ipmiblob
{
namespace internal
{

class InternalSysMock : public Sys
{
  public:
    virtual ~InternalSysMock() = default;

    MOCK_METHOD(int, open, (const char*, int), (const, override));
    MOCK_METHOD(int, read, (int, void*, std::size_t), (const, override));
    MOCK_METHOD(int, close, (int), (const, override));
    MOCK_METHOD(void*, mmap, (void*, std::size_t, int, int, int, off_t),
                (const, override));
    MOCK_METHOD(int, munmap, (void*, std::size_t), (const, override));
    MOCK_METHOD(int, getpagesize, (), (const, override));
    MOCK_METHOD(int, ioctl, (int, unsigned long, void*), (const, override));
    MOCK_METHOD(int, poll, (struct pollfd*, nfds_t, int), (const, override));
};

} // namespace internal
} // namespace ipmiblob
