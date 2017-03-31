#include <iostream>
#include <stdexcept>
#include <array>
#include "sbe_interfaces.hpp"

namespace openpower
{
namespace sbe
{

//Helper interfaces
static inline uint32_t upper(uint64_t value)
{
    return ((value & 0xFFFFFFFF00000000ull) >> 32);
}

static inline uint32_t lower(uint64_t value)
{
    return (value & 0xFFFFFFFF);
}

using sbe_word_t = uint32_t;

namespace scom
{

//Constants specific to SCOM operations
static constexpr sbe_word_t READ_OPCODE  = 0x0000A201;
static constexpr sbe_word_t WRITE_OPCODE = 0x0000A202;
static constexpr size_t READ_CMD_LENGTH = 0x4;
static constexpr size_t WRITE_CMD_LENGTH = 0x6;

//Reading SCOM Registers
uint64_t read(const char* devPath,
              uint64_t address)
{
    uint64_t value = 0;

    //Validate input device path
    if (devPath == nullptr)
    {
        throw std::runtime_error("NULL FIFO device path");
    }

    //Build SCOM read request command
    std::array<sbe_word_t, READ_CMD_LENGTH> command =
    {
        static_cast<sbe_word_t>(READ_CMD_LENGTH),
        READ_OPCODE,
        upper(address),
        lower(address)
    };

    std::cout << "Size of read command buffer:" << command.size();

    // TODO: Call an interface to read the command to the SBE FIFO and read the
    // response from the SBE FIFO device

    return value;
}

void write(const char* devPath,
           uint64_t address,
           uint64_t data)
{
    //Validate input device path
    if (devPath == nullptr)
    {
        throw std::runtime_error("NULL FIFO device path");
    }

    //Build SCOM write request command
    std::array<sbe_word_t, WRITE_CMD_LENGTH> command =
    {
        static_cast<sbe_word_t>(WRITE_CMD_LENGTH),
        WRITE_OPCODE,
        upper(address),
        lower(address),
        upper(data),
        lower(data)
    };

    std::cout << "Size of write command buffer:" << command.size();
 
    // TODO: Call an interface to write the command to the SBE FIFO and read the
    // response from the SBE FIFO device

}

} // namespace scom
} // namespace sbe
} // namespace openpower
