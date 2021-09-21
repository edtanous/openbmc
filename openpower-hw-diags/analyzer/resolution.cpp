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

// Helper function to get the connected target on the other side of the
// given bus.
pdbg_target* __getConnectedTarget(pdbg_target* i_rxTarget,
                                  const callout::BusType& i_busType)
{
    assert(nullptr != i_rxTarget);

    pdbg_target* txTarget = nullptr;

    auto rxType        = util::pdbg::getTrgtType(i_rxTarget);
    std::string rxPath = util::pdbg::getPath(i_rxTarget);

    if (callout::BusType::SMP_BUS == i_busType &&
        util::pdbg::TYPE_IOLINK == rxType)
    {
        // TODO: Will need to reference some sort of data that can tell us how
        //       the processors are connected in the system. For now, return the
        //       RX target to avoid returning a nullptr.
        trace::inf("No support to get peer target on SMP bus");
        txTarget = i_rxTarget;
    }
    else if (callout::BusType::OMI_BUS == i_busType &&
             util::pdbg::TYPE_OMI == rxType)
    {
        // This is a bit clunky. The pdbg APIs only give us the ability to
        // iterate over the children instead of just returning a list. So we'll
        // push all the children to a list and go from there.
        std::vector<pdbg_target*> childList;

        pdbg_target* childTarget = nullptr;
        pdbg_for_each_target("ocmb", i_rxTarget, childTarget)
        {
            if (nullptr != childTarget)
            {
                childList.push_back(childTarget);
            }
        }

        // We know there should only be one OCMB per OMI.
        if (1 != childList.size())
        {
            throw std::logic_error("Invalid child list size for " + rxPath);
        }

        // Get the connected target.
        txTarget = childList.front();
    }
    else if (callout::BusType::OMI_BUS == i_busType &&
             util::pdbg::TYPE_OCMB == rxType)
    {
        txTarget = pdbg_target_parent("omi", i_rxTarget);
        if (nullptr == txTarget)
        {
            throw std::logic_error("No parent OMI found for " + rxPath);
        }
    }
    else
    {
        // This would be a code bug.
        throw std::logic_error("Unsupported config: i_rxTarget=" + rxPath +
                               " i_busType=" + i_busType.getString());
    }

    assert(nullptr != txTarget); // just in case we missed something above

    return txTarget;
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

void ConnectedCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the chip target from the root cause signature.
    auto chipTarget = __getRootCauseChipTarget(io_sd);

    // Get the endpoint target for the receiving side of the bus.
    auto rxTarget = __getUnitTarget(chipTarget, iv_unitPath);

    // Get the endpoint target for the transfer side of the bus.
    auto txTarget = __getConnectedTarget(rxTarget, iv_busType);

    // Callout the TX endpoint.
    nlohmann::json txCallout;
    txCallout["LocationCode"] = util::pdbg::getLocationCode(txTarget);
    txCallout["Priority"]     = iv_priority.getUserDataString();
    io_sd.addCallout(txCallout);

    // Guard the TX endpoint.
    Guard txGuard =
        io_sd.addGuard(util::pdbg::getPhysDevPath(txTarget), iv_guard);

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Connected Callout";
    ffdc["Bus Type"]     = iv_busType.getString();
    ffdc["Target"]       = util::pdbg::getPhysDevPath(txTarget);
    ffdc["Priority"]     = iv_priority.getRegistryString();
    ffdc["Guard Type"]   = txGuard.getString();
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void BusCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the chip target from the root cause signature.
    auto chipTarget = __getRootCauseChipTarget(io_sd);

    // Get the endpoint target for the receiving side of the bus.
    auto rxTarget = __getUnitTarget(chipTarget, iv_unitPath);

    // Get the endpoint target for the transfer side of the bus.
    auto txTarget = __getConnectedTarget(rxTarget, iv_busType);

    // Callout the RX endpoint.
    nlohmann::json rxCallout;
    rxCallout["LocationCode"] = util::pdbg::getLocationCode(rxTarget);
    rxCallout["Priority"]     = iv_priority.getUserDataString();
    io_sd.addCallout(rxCallout);

    // Callout the TX endpoint.
    nlohmann::json txCallout;
    txCallout["LocationCode"] = util::pdbg::getLocationCode(txTarget);
    txCallout["Priority"]     = iv_priority.getUserDataString();
    io_sd.addCallout(txCallout);

    // Callout everything else in between.
    // TODO: For P10 (OMI bus and XBUS), the callout is simply the backplane.
    //       There isn't a devtree object for this, yet. So will need to
    //       hardcode the location code for now. In the future, we will need a
    //       mechanism to make this data driven.
    nlohmann::json bpCallout;
    bpCallout["LocationCode"] = "P0";
    bpCallout["Priority"]     = iv_priority.getUserDataString();
    io_sd.addCallout(bpCallout);

    // Guard the RX endpoint.
    Guard guard =
        io_sd.addGuard(util::pdbg::getPhysDevPath(rxTarget), iv_guard);

    // Guard the TX endpoint.
    // No need to check return because it is the same as RX target.
    io_sd.addGuard(util::pdbg::getPhysDevPath(txTarget), iv_guard);

    // TODO: Currently no guard for "everything else in between".

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Bus Callout";
    ffdc["Bus Type"]     = iv_busType.getString();
    ffdc["RX Target"]    = util::pdbg::getPhysDevPath(rxTarget);
    ffdc["TX Target"]    = util::pdbg::getPhysDevPath(txTarget);
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
