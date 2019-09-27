
#include <isolator/hei_isolator.hpp>
#include <register/hei_hardware_register.hpp>
#include <util/hei_flyweight.hpp>

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
    // Must flush the hardware register cache before deleting any
    // HardwareRegister objects.
    HardwareRegister::flushAll();

    // BEGIN temporary code
    HEI_INF( "Isolator::uninitialize()" );
    // END temporary code

    // Remove all of the isolation objects stored in the flyweights.
    Flyweight<HardwareRegister>::getSingleton().clear();
}

ReturnCode Isolator::isolate( const std::vector<Chip> & i_chipList,
                              IsolationData & o_isoData ) const
{
    ReturnCode rc;

    // Flush the isolation data to ensure a clean slate.
    o_isoData.clear();

    // Flush the hardware register cache to avoid using stale data.
    HardwareRegister::flushAll();

    // Analyze active error on each chip.
    for ( auto const & chip : i_chipList )
    {
        // In order to access hardware, we must tell the HardwareRegisters which
        // chip to access.
        HardwareRegister::setAccessor( chip );

        // BEGIN temporary code
        HEI_INF( "Isolator::isolate(%p,%u)", chip.getChip(),
                 chip.getType() );
        // END temporary code

        // Clean up the hardware accessor chip to prevent accidental hardware
        // access.
        HardwareRegister::clearAccessor();
    }

    return rc;
}

} // end namespace libhei
