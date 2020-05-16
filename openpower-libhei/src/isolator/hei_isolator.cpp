
#include <hei_includes.hpp>
#include <isolator/hei_isolation_node.hpp>
#include <isolator/hei_isolator.hpp>
#include <register/hei_operator_register.hpp>
#include <register/hei_scom_register.hpp>
#include <util/hei_flyweight.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

// Definitions for the interfaces defined in hei_main.hpp.

void initialize(void* i_buffer, size_t i_bufferSize)
{
    Isolator::getSingleton().initialize(i_buffer, i_bufferSize);
}

void uninitialize()
{
    Isolator::getSingleton().uninitialize();
}

void isolate(const std::vector<Chip>& i_chipList, IsolationData& o_isoData)
{
    Isolator::getSingleton().isolate(i_chipList, o_isoData);
}

//------------------------------------------------------------------------------

void Isolator::uninitialize()
{
    // Clear all of the isolation objects.
    iv_isoChips.clear();

    // Must flush the hardware register cache before deleting any
    // HardwareRegister objects.
    HardwareRegister::flushAll();

    // Clear the operator register flyweights.
    Flyweight<const ConstantRegister>::getSingleton().clear();
    Flyweight<const AndRegister>::getSingleton().clear();
    Flyweight<const OrRegister>::getSingleton().clear();
    Flyweight<const NotRegister>::getSingleton().clear();
    Flyweight<const LeftShiftRegister>::getSingleton().clear();
    Flyweight<const RightShiftRegister>::getSingleton().clear();

    // Remove all of the HardwareRegister objects stored in the flyweights.
    Flyweight<const ScomRegister>::getSingleton().clear();
    Flyweight<const IdScomRegister>::getSingleton().clear();
}

void Isolator::isolate(const std::vector<Chip>& i_chipList,
                       IsolationData& o_isoData) const
{
    // Flush the isolation data to ensure a clean slate.
    o_isoData.flush();

    // Flush the hardware register cache to avoid using stale data.
    HardwareRegister::flushAll();

    // Analyze active error on each chip.
    for (const auto& chip : i_chipList)
    {
        // Isolation objects for this chip's type must exist.
        const auto& itr = iv_isoChips.find(chip.getType());
        HEI_ASSERT(iv_isoChips.end() != itr);

        // Analyze this chip.
        itr->second->analyze(chip, o_isoData);
    }
}

} // end namespace libhei
