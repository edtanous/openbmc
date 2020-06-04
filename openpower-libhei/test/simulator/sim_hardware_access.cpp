/**
 * @file These are the simulated implementations of the user interfaces declared
 *       in hei_user_interface.hpp
 */

#include "simulator.hpp"

#include <endian.h>

#include <hei_includes.hpp>
#include <hei_user_interface.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

bool registerRead(const Chip& i_chip, RegisterType_t i_regType,
                  uint64_t i_address, uint64_t& o_value)
{
    bool accessFailure = false;

    // Get access to data through the singleton.
    SimulatorData& theSimData = SimulatorData::getSingleton();

    switch (i_regType)
    {
        case REG_TYPE_SCOM:
            o_value = theSimData.getScomReg(i_chip, (uint32_t)i_address);
            break;
        case REG_TYPE_ID_SCOM:
            o_value = theSimData.getIdScomReg(i_chip, i_address);
            break;
        default:
            accessFailure = true;
            HEI_ERR("registerRead(%p,%" PRIu8 ",%" PRIx64 ")", i_chip.getChip(),
                    i_regType, i_address);
    }

    return accessFailure;
}

//------------------------------------------------------------------------------

#ifdef __HEI_ENABLE_HW_WRITE

bool registerWrite(const Chip& i_chip, RegisterType_t i_regType,
                   uint64_t i_address, uint64_t i_value)
{
    bool accessFailure = false;

    switch (i_regType)
    {
        // TODO: add cases for REG_TYPE_SCOM and REG_TYPE_ID_SCOM
        default:
            accessFailure = true;
            HEI_ERR("registerWrite(%p,%" PRIu8 ",%" PRIx64 ",%" PRIx64 ")",
                    i_chip.getChip(), i_regType, i_address, i_value);
    }

    return accessFailure;
}

#endif // __HEI_ENABLE_HW_WRITE

//------------------------------------------------------------------------------

} // namespace libhei
