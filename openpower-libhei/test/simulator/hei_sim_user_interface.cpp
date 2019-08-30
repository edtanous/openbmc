#include <hei_user_interface.hpp>

#include <hei_includes.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

ReturnCode registerRead( void * i_chip, void * o_buffer, size_t & io_bufSize,
                         uint64_t i_regType, uint64_t i_address )
{
    ReturnCode rc;

    HEI_ASSERT( nullptr != i_chip );
    HEI_ASSERT( nullptr != o_buffer );
    HEI_ASSERT( 0 != io_bufSize );

    switch ( i_regType )
    {
        default:
            rc = RC_REG_ACCESS_FAILURE;
            HEI_ERR( "registerRead(%p,%p,%lx,%lx,%lx)", i_chip, o_buffer,
                     io_bufSize, i_regType, i_address );
    }

    return rc;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

ReturnCode registerWrite( void * i_chip, void * i_buffer, size_t & io_bufSize,
                          uint64_t i_regType, uint64_t i_address )
{
    ReturnCode rc;

    HEI_ASSERT( nullptr != i_chip );
    HEI_ASSERT( nullptr != i_buffer );
    HEI_ASSERT( 0 != io_bufSize );

    switch ( i_regType )
    {
        default:
            rc = RC_REG_ACCESS_FAILURE;
            HEI_ERR( "registerWrite(%p,%p,%lx,%lx,%lx)", i_chip, i_buffer,
                     io_bufSize, i_regType, i_address );
    }

    return rc;
}

#endif

//------------------------------------------------------------------------------

} // end namespace libhei

