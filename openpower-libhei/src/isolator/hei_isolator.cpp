
#include <isolator/hei_isolator.hpp>

namespace libhei
{

ReturnCode Isolator::initialize( void * i_buffer, size_t i_bufferSize,
                                 bool i_forceInit )
{
    ReturnCode rc;

    // BEGIN temporary code
    HEI_INF( "Isolator::initialize(%p,%lu,%d)", i_buffer, i_bufferSize,
             i_forceInit );
    // END temporary code

    return rc;
}

void Isolator::uninitialize()
{
    // BEGIN temporary code
    HEI_INF( "Isolator::uninitialize()" );
    // END temporary code
}

ReturnCode Isolator::isolate( IsolationData & o_isoData ) const
{
    ReturnCode rc;

    // Flush the isolation data to ensure a clean slate.
    o_isoData.clear();

    // BEGIN temporary code
    HEI_INF( "Isolator::isolate(%p,%u)", o_isoData.getChip(),
             o_isoData.getChipType() );
    // END temporary code

    return rc;
}

} // end namespace libhei
