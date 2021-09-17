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

    // Get the location code and entity path for this target.
    auto locCode    = util::pdbg::getLocationCode(target);
    auto entityPath = util::pdbg::getPhysDevPath(target);

    // Add the actual callout to the service data.
    nlohmann::json callout;
    callout["LocationCode"] = locCode;
    callout["Priority"]     = iv_priority.getUserDataString();
    io_sd.addCallout(callout);

    // Add the guard info to the service data.
    Guard guard = io_sd.addGuard(entityPath, iv_guard);

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Hardware Callout";
    ffdc["Target"]       = entityPath;
    ffdc["Priority"]     = iv_priority.getRegistryString();
    ffdc["Guard Type"]   = guard.getString();
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ClockCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Add the callout to the service data.
    // TODO: For P10, the callout is simply the backplane. There isn't a devtree
    //       object for this, yet. So will need to hardcode the location code
    //       for now. In the future, we will need a mechanism to make this data
    //       driven.
    nlohmann::json callout;
    callout["LocationCode"] = "P0";
    callout["Priority"]     = iv_priority.getUserDataString();
    io_sd.addCallout(callout);

    // Add the guard info to the service data.
    // TODO: Still waiting for clock targets to be defined in the device tree.
    //       For get the processor path for the FFDC.
    // static const std::map<callout::ClockType, std::string> m = {
    //     {callout::ClockType::OSC_REF_CLOCK_0, ""},
    //     {callout::ClockType::OSC_REF_CLOCK_1, ""},
    // };
    // auto target = std::string{util::pdbg::getPath(m.at(iv_clockType))};
    // auto guardPath = util::pdbg::getPhysDevPath(target);
    // Guard guard = io_sd.addGuard(guardPath, iv_guard);
    auto target    = __getRootCauseChipTarget(io_sd);
    auto guardPath = util::pdbg::getPhysDevPath(target);

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Clock Callout";
    ffdc["Clock Type"]   = iv_clockType.getString();
    ffdc["Target"]       = guardPath;
    ffdc["Priority"]     = iv_priority.getRegistryString();
    ffdc["Guard Type"]   = ""; // TODO: guard.getString();
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ProcedureCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Add the actual callout to the service data.
    nlohmann::json callout;
    callout["Procedure"] = iv_procedure.getString();
    callout["Priority"]  = iv_priority.getUserDataString();
    io_sd.addCallout(callout);

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Procedure Callout";
    ffdc["Procedure"]    = iv_procedure.getString();
    ffdc["Priority"]     = iv_priority.getRegistryString();
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

} // namespace analyzer
