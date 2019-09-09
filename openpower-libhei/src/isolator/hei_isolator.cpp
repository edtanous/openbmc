
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

ReturnCode Isolator::isolate( const std::vector<Chip> & i_chipList,
                              IsolationData & o_isoData ) const
{
    ReturnCode rc;

    // Flush the isolation data to ensure a clean slate.
    o_isoData.clear();

    // BEGIN temporary code
    for ( auto const & chip : i_chipList )
    {
        HEI_INF( "Isolator::isolate(%p,%u)", chip.getChip(),
                 chip.getType() );
    }
    // END temporary code

    return rc;
}

} // end namespace libhei
