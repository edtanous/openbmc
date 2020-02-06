/**
 * @file These are implementations of the user interfaces declared in
 *       hei_user_interface.hpp
 */

#include <hei_user_interface.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

ReturnCode registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                        uint64_t i_regType, uint64_t i_address)
{
    ReturnCode rc = RC_SUCCESS;

    //    HEI_INF("registerRead(%p,%p,%lx,%lx,%lx)", i_chip.getChip(),
    //             o_buffer, io_bufSize, i_regType, i_address);

    return rc;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

ReturnCode registerWrite(const Chip& i_chip, void* i_buffer, size_t& io_bufSize,
                         uint64_t i_regType, uint64_t i_address)
{
    ReturnCode rc = RC_SUCCESS;

    //    HEI_INF("registerWrite(%p,%p,%lx,%lx,%lx)", i_chip.getChip(),
    //             i_buffer, io_bufSize, i_regType, i_address);

    return rc;
}

#endif

//------------------------------------------------------------------------------

} // namespace libhei
