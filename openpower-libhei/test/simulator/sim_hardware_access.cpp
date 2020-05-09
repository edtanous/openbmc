/**
 * @file These are the simulated implementations of the user interfaces declared
 *       in hei_user_interface.hpp
 */

#include "simulator.hpp"

#include <endian.h>

#include <hei_macros.hpp>
#include <hei_user_interface.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

bool registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                  uint64_t i_regType, uint64_t i_address)
{
    bool accessFailure = false;

    HEI_ASSERT(nullptr != o_buffer);
    HEI_ASSERT(0 != io_bufSize);

    // Get access to data through the singleton
    SimulatorData& theSimData = SimulatorData::getSingleton();

    switch (i_regType)
    {
        case REG_TYPE_SCOM:
        {
            // Get the register value and change its endianness
            uint64_t regValue =
                htobe64(theSimData.getScomReg(i_chip, (uint32_t)i_address));
            // Get size of register value for calling code and memcopy
            io_bufSize = sizeof(regValue);
            memcpy(o_buffer, &regValue, io_bufSize);
            break;
        }
        case REG_TYPE_ID_SCOM:
        {
            // Get the register value and change its endianness
            uint64_t regValue =
                htobe64(theSimData.getIdScomReg(i_chip, i_address));
            // Get size of register value for calling code and memcopy
            io_bufSize = sizeof(regValue);
            memcpy(o_buffer, &regValue, io_bufSize);
            break;
        }
        default:
            accessFailure = true;
            HEI_ERR("registerRead(%p,%p,%" PRIu64 ",%" PRIx64 ",%" PRIx64 ")",
                    i_chip.getChip(), o_buffer, (uint64_t)io_bufSize, i_regType,
                    i_address);
    }

    return accessFailure;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

bool registerWrite(const Chip& i_chip, void* i_buffer, size_t& io_bufSize,
                   uint64_t i_regType, uint64_t i_address)
{
    bool accessFailure = false;

    HEI_ASSERT(nullptr != i_buffer);
    HEI_ASSERT(0 != io_bufSize);

    switch (i_regType)
    {
        // TODO: add cases for REG_TYPE_SCOM and REG_TYPE_ID_SCOM
        default:
            accessFailure = true;
            HEI_ERR("registerWrite(%p,%p,%" PRIu64 ",%" PRIx64 ",%" PRIx64 ")",
                    i_chip.getChip(), i_buffer, (uint64_t)io_bufSize, i_regType,
                    i_address);
    }

    return accessFailure;
}

#endif

//------------------------------------------------------------------------------

} // namespace libhei
