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

    // Add location code to callout list.
    auto locCode = util::pdbg::getLocationCode(trgt);
    if (locCode.empty())
    {
        trace::err("Unable to find location code for %s", path.c_str());
    }
    else
    {
        io_sd.addCallout(
            std::make_shared<HardwareCallout>(locCode, iv_priority));
    }

    // Add entity path to gard list.
    auto entityPath = util::pdbg::getPhysDevPath(trgt);
    if (entityPath.empty())
    {
        trace::err("Unable to find entity path for %s", path.c_str());
    }
    else
    {
        io_sd.addGuard(std::make_shared<Guard>(entityPath, iv_guard));
    }
}

} // namespace analyzer
