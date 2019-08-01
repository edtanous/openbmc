
#include "hei_isolator.hpp"

namespace libhei
{

ReturnCode Isolator::initialize( void * i_buffer, size_t i_bufferSize,
                                 bool i_forceInit )
{
    ReturnCode rc;

    // BEGIN temporary code for compilation
    HEI_INF( "Isolator::initialize(%p,%lu,%d)", i_buffer, i_bufferSize,
             i_forceInit );
    // END temporary code for compilation

    return rc;
}

ReturnCode Isolator::isolate( void * i_chip, ChipType_t i_chipType,
                              IsolationData & o_isoData )
{
    ReturnCode rc;

    // BEGIN temporary code for compilation
    o_isoData = IsolationData();
    HEI_INF( "Isolator::isolate(%p,%u)", i_chip, i_chipType );
    // END temporary code for compilation

    return rc;
}

} // end namespace libhei
