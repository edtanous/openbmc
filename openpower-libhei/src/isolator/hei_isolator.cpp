
#include <hei_includes.hpp>
#include <isolator/hei_isolation_node.hpp>
#include <isolator/hei_isolator.hpp>
#include <register/hei_hardware_register.hpp>
#include <util/hei_flyweight.hpp>

// BEGIN temporary code
#include <isolator/hei_isolation_node.hpp>
#include <register/hei_operator_register.hpp>
#include <register/hei_scom_register.hpp>
// END temporary code

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

void Isolator::initialize(void* i_buffer, size_t i_bufferSize)
{
    // TODO
}

void Isolator::uninitialize()
{
    // Remove all of the IsolationNode objects stored in the flyweights. This
    // must be done before removing the HardwareRegister objects
    Flyweight<IsolationNode>::getSingleton().clear();

    // Must flush the hardware register cache before deleting any
    // HardwareRegister objects.
    HardwareRegister::flushAll();

    // Remove all of the HardwareRegister objects stored in the flyweights.
    Flyweight<ScomRegister>::getSingleton().clear();
    Flyweight<IdScomRegister>::getSingleton().clear();
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
        // TODO
        HEI_INF("Isolator::isolate(%p,%u)", chip.getChip(), chip.getType());
    }
}

} // end namespace libhei
