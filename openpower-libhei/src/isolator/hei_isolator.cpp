
#include <isolator/hei_isolator.hpp>
#include <register/hei_scom_register.hpp>
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

    Flyweight<ScomRegister>   & sfw = Flyweight<ScomRegister>::getSingleton();
    Flyweight<IdScomRegister> & ifw = Flyweight<IdScomRegister>::getSingleton();

    sfw.get( ScomRegister { CHIP_TYPE_INVALID, REG_ID_INVALID,
                            REG_INST_DEFAULT, REG_ACCESS_RW, 0x01234567 } );
    sfw.get( ScomRegister { CHIP_TYPE_INVALID, REG_ID_INVALID,
                            REG_INST_DEFAULT, REG_ACCESS_RW, 0x00112233 } );

    ifw.get( IdScomRegister { CHIP_TYPE_INVALID, REG_ID_INVALID,
                              REG_INST_DEFAULT, REG_ACCESS_RW,
                              0x0123456789abcdef } );
    ifw.get( IdScomRegister { CHIP_TYPE_INVALID, REG_ID_INVALID,
                              REG_INST_DEFAULT, REG_ACCESS_RW,
                              0x0011223344556677 } );
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
    Flyweight<ScomRegister>::getSingleton().clear();
    Flyweight<IdScomRegister>::getSingleton().clear();
}

ReturnCode Isolator::isolate( const std::vector<Chip> & i_chipList,
                              IsolationData & o_isoData ) const
{
    ReturnCode rc;

    // Flush the isolation data to ensure a clean slate.
    o_isoData.flush();

    // Flush the hardware register cache to avoid using stale data.
    HardwareRegister::flushAll();

    // Analyze active error on each chip.
    for ( auto const & chip : i_chipList )
    {
        // BEGIN temporary code
        HEI_INF( "Isolator::isolate(%p,%u)", chip.getChip(),
                 chip.getType() );
        // END temporary code
    }

    return rc;
}

} // end namespace libhei
