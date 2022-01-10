#include <analyzer/plugins/plugin.hpp>
#include <analyzer/resolution.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

//------------------------------------------------------------------------------

// Helper function to get the root cause chip target from the service data.
pdbg_target* __getRootCauseChipTarget(const ServiceData& i_sd)
{
    auto target = util::pdbg::getTrgt(i_sd.getRootCause().getChip());
    assert(nullptr != target); // This would be a really bad bug.
    return target;
}

//------------------------------------------------------------------------------

// Helper function to get a unit target from the given unit path, which is a
// devtree path relative the the containing chip. An empty string indicates the
// chip target should be returned.
pdbg_target* __getUnitTarget(pdbg_target* i_chipTarget,
                             const std::string& i_unitPath)
{
    assert(nullptr != i_chipTarget);

    auto target = i_chipTarget; // default, if i_unitPath is empty

    if (!i_unitPath.empty())
    {
        auto path = std::string{util::pdbg::getPath(target)} + "/" + i_unitPath;

        target = util::pdbg::getTrgt(path);
        if (nullptr == target)
        {
            // Likely a bug the RAS data files.
            throw std::logic_error("Unable to find target for " + path);
        }
    }

    return target;
}

//------------------------------------------------------------------------------

void HardwareCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the target for the hardware callout.
    auto target = __getUnitTarget(__getRootCauseChipTarget(io_sd), iv_unitPath);

    // Add the callout and the FFDC to the service data.
    io_sd.calloutTarget(target, iv_priority, iv_guard);
}

//------------------------------------------------------------------------------

void ConnectedCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the chip target from the root cause signature.
    auto chipTarget = __getRootCauseChipTarget(io_sd);

    // Get the endpoint target for the receiving side of the bus.
    auto rxTarget = __getUnitTarget(chipTarget, iv_unitPath);

    // Add the callout and the FFDC to the service data.
    io_sd.calloutConnected(rxTarget, iv_busType, iv_priority, iv_guard);
}

//------------------------------------------------------------------------------

void BusCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the chip target from the root cause signature.
    auto chipTarget = __getRootCauseChipTarget(io_sd);

    // Get the endpoint target for the receiving side of the bus.
    auto rxTarget = __getUnitTarget(chipTarget, iv_unitPath);

    // Add the callout and the FFDC to the service data.
    io_sd.calloutBus(rxTarget, iv_busType, iv_priority, iv_guard);
}

//------------------------------------------------------------------------------

void ClockCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Add the callout and the FFDC to the service data.
    io_sd.calloutClock(iv_clockType, iv_priority, iv_guard);
}

//------------------------------------------------------------------------------

void ProcedureCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Add the callout and the FFDC to the service data.
    io_sd.calloutProcedure(iv_procedure, iv_priority);
}

//------------------------------------------------------------------------------

void PluginResolution::resolve(ServiceData& io_sd) const
{
    // Get the plugin function and call it.

    auto chip = io_sd.getRootCause().getChip();

    auto func = PluginMap::getSingleton().get(chip.getType(), iv_name);

    func(iv_instance, chip, io_sd);
}

//------------------------------------------------------------------------------

} // namespace analyzer
