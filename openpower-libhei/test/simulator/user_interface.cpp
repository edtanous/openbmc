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

    switch (i_regType)
    {
        // BEGIN temporary code
        // TODO: add cases for REG_TYPE_SCOM and REG_TYPE_ID_SCOM
        case REG_TYPE_SCOM:
        {
            uint64_t x = htobe64(0x8800000000000000);
            memcpy(o_buffer, &x, sizeof(x));
            break;
        }
        case REG_TYPE_ID_SCOM:
        {
            uint64_t x = htobe64(0x8000);
            memcpy(o_buffer, &x, sizeof(x));
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
