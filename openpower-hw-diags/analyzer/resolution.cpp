#include <analyzer/resolution.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

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
    io_sd.addGuard(entityPath, iv_guard);
}

//------------------------------------------------------------------------------

void ProcedureCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Add the actual callout to the service data.
    nlohmann::json callout;
    callout["Procedure"] = iv_procedure.getString();
    callout["Priority"]  = iv_priority.getUserDataString();
    io_sd.addCallout(callout);
}

//------------------------------------------------------------------------------

} // namespace analyzer
