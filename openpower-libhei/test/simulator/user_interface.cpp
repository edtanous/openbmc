/**
 * @file These are the simulated implementations of the user interfaces declared
 *       in hei_user_interface.hpp
 */

#include "simulator.hpp"

#include <endian.h>

#include <hei_user_interface.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

ReturnCode registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                        uint64_t i_regType, uint64_t i_address)
{
    ReturnCode rc{};

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
        // END temporary code
        default:
            rc = RC_REG_ACCESS_FAILURE;
            HEI_ERR("registerRead(%p,%p,%lx,%lx,%lx)", i_chip.getChip(),
                    o_buffer, io_bufSize, i_regType, i_address);
    }

    return rc;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

ReturnCode registerWrite(const Chip& i_chip, void* i_buffer, size_t& io_bufSize,
                         uint64_t i_regType, uint64_t i_address)
{
    ReturnCode rc{};

    HEI_ASSERT(nullptr != i_buffer);
    HEI_ASSERT(0 != io_bufSize);

    switch (i_regType)
    {
        // TODO: add cases for REG_TYPE_SCOM and REG_TYPE_ID_SCOM
        default:
            rc = RC_REG_ACCESS_FAILURE;
            HEI_ERR("registerWrite(%p,%p,%lx,%lx,%lx)", i_chip.getChip(),
                    i_buffer, io_bufSize, i_regType, i_address);
    }

    return rc;
}

#endif

//------------------------------------------------------------------------------

} // namespace libhei
