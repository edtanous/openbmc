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

    // Get the location code for this target.
    auto locCode = util::pdbg::getLocationCode(trgt);

    // Add the actual callout to the service data.
    nlohmann::json callout;
    callout["LocationCode"] = locCode;
    callout["Priority"]     = iv_priority.getUserDataString();
    io_sd.addCallout(callout);

    // Add entity path to gard list.
    auto entityPath = util::pdbg::getPhysDevPath(trgt);
    if (entityPath.empty())
    {
        trace::err("Unable to find entity path for %s", path.c_str());
    }
    else
    {
        Guard::Type guard = Guard::NONE;
        if (iv_guard)
        {
            guard = io_sd.queryCheckstop() ? Guard::FATAL : Guard::NON_FATAL;
        }

        io_sd.addGuard(std::make_shared<Guard>(entityPath, guard));
    }
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
