#include <iostream>
#include <stdexcept>
#include <array>
#include <endian.h>
#include "sbe_interfaces.hpp"
#include "sbe_chipOp_handler.hpp"

namespace openpower
{
namespace sbe
{

constexpr size_t RESP_HEADER_LEN = 0x3;

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
static constexpr size_t READ_RESP_LENGTH = 0x2;

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

    //Build SCOM read request command.
    //Handle byte order mismatch ,SBE is big endian and BMC is
    //little endian.
    std::array<sbe_word_t, READ_CMD_LENGTH> command =
    {
        static_cast<sbe_word_t>(htobe32(READ_CMD_LENGTH)),
        htobe32(READ_OPCODE),
        htobe32(upper(address)),
        htobe32(lower(address))
    };

    //Buffer to hold the response data along with the SBE header
    const size_t respLength = RESP_HEADER_LEN + READ_RESP_LENGTH ;
    std::array<sbe_word_t, respLength> response = {};

    //Write the command buffer to the SBE FIFO and obtain the response from the
    //SBE FIFO device.This interface will parse the obtained SBE response and
    //any internal SBE failures will be communicated via exceptions
    invokeSBEChipOperation(devPath, command, response);

    value = (((static_cast<uint64_t>(response[0])) << 32) | response[1]);
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
    //Handle byte order mismatch, SBE is big endian and BMC is
    //little endian.
    std::array<sbe_word_t, WRITE_CMD_LENGTH> command =
    {
        static_cast<sbe_word_t>(htobe32(WRITE_CMD_LENGTH)),
        htobe32(WRITE_OPCODE),
        htobe32(upper(address)),
        htobe32(lower(address)),
        htobe32(upper(data)),
        htobe32(lower(data))
    };

    //Buffer to hold the SBE response status
    const size_t respLength = RESP_HEADER_LEN;
    std::array<sbe_word_t, respLength> response = {};

    //Write the command buffer to the SBE FIFO and obtain the response from the
    //SBE FIFO device.This interface will parse the obtained SBE response and
    //any internal SBE failures will be communicated via exceptions
    invokeSBEChipOperation(devPath, command, response);
}

} // namespace scom
} // namespace sbe
} // namespace openpower
