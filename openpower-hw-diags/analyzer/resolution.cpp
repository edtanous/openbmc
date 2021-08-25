#include <analyzer/resolution.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

//------------------------------------------------------------------------------

void HardwareCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the chip target from the root cause signature.
    auto trgt = util::pdbg::getTrgt(io_sd.getRootCause().getChip());
    auto path = std::string{util::pdbg::getPath(trgt)};

    // Get the unit target, if needed.
    if (!iv_path.empty())
    {
        path += "/" + iv_path;
        trgt = util::pdbg::getTrgt(path);
        if (nullptr == trgt)
        {
            trace::err("Unable to find target for %s", path.c_str());
            return; // can't continue
        }
    }

    // Get the location code and entity path for this target.
    auto locCode    = util::pdbg::getLocationCode(trgt);
    auto entityPath = util::pdbg::getPhysDevPath(trgt);

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
