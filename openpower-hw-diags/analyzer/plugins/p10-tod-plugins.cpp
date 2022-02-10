
#include <analyzer/plugins/plugin.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

namespace P10
{

namespace tod
{

/** Each chip is connected to two TOD topologies: active and backup. The values
 *  are important because some registers and documentation simply refer to them
 *  by number instead of name. Also, they can be used as array indexes if
 *  needed. */
enum class Topology
{
    ACTIVE = 0,
    BACKUP = 1,
};

class Data
{
  public:
    Data()            = default;
    ~Data()           = default;
    Data(const Data&) = default;
    Data(Data&&)      = default;
    Data& operator=(const Data&) = default;
    Data& operator=(Data&&) = default;

  private:
    /** The MDMT chips at fault (only one per topology). */
    std::map<Topology, pdbg_target*> iv_mdmtFaultList;

    /** All chips with internal path faults. */
    std::map<Topology, std::vector<pdbg_target*>> iv_internalFaultList;

    /** The chips sourcing the clocks to non-MDMT chips with faults. */
    std::map<Topology, std::vector<pdbg_target*>> iv_networkFaultList;

  public:
    /**
     * @brief Sets this chip as the MDMT at fault for this topology.
     * @param i_topology    Target topology.
     * @param i_chipAtFault The chip reporting step check fault.
     */
    void setMdmtFault(Topology i_topology, pdbg_target* i_chipAtFault)
    {
        assert(nullptr != i_chipAtFault);
        iv_mdmtFaultList[i_topology] = i_chipAtFault;
    }

    /**
     * @param  i_topology Target topology.
     * @return The MDMT chip for this topology, if at fault. Otherwise, nullptr.
     */
    pdbg_target* getMdmtFault(Topology i_topology)
    {
        return iv_mdmtFaultList[i_topology];
    }

    /**
     * @brief Indicates the given chip has an internal fault.
     * @param i_topology    Target topology.
     * @param i_chipAtFault The chip reporting a step check fault.
     */
    void setInternalFault(Topology i_topology, pdbg_target* i_chipAtFault)
    {
        assert(nullptr != i_chipAtFault);
        iv_internalFaultList[i_topology].push_back(i_chipAtFault);
    }

    /**
     * @param  i_topology Target topology.
     * @return The list of all chips with internal faults.
     */
    const std::vector<pdbg_target*>& getInteralFaults(Topology i_topology)
    {
        return iv_internalFaultList[i_topology];
    }

    /**
     * @brief Indicates the given non-MDMT chip has seen a fault in the TOD
     *        network.
     * @param i_topology          Target topology.
     * @param i_chipSourcingClock The chip sourcing the clock for the chip at
     *                            fault. This is NOT the chip at fault.
     */
    void setNetworkFault(Topology i_topology, pdbg_target* i_chipSourcingClock)
    {
        assert(nullptr != i_chipSourcingClock);
        iv_networkFaultList[i_topology].push_back(i_chipSourcingClock);
    }

    /**
     * @param  i_topology Target topology.
     * @return The list of all chips sourcing the clocks for the non-MDMT chips
     *         with step check faults.
     */
    const std::vector<pdbg_target*>& getNetworkFaults(Topology i_topology)
    {
        return iv_networkFaultList[i_topology];
    }
};

/**
 * @brief Collects TOD fault data for each processor chip.
 */
void collectTodFaultData(pdbg_target*, Data&)
{
    // TODO: Need to query hardware for this chip and add to Data if any
    // faults found.
}

} // namespace tod

/**
 * @brief Handles TOD step check fault attentions.
 */
void tod_step_check_fault(unsigned int, const libhei::Chip& i_chip,
                          ServiceData& io_servData)
{
    // Query hardware for TOD fault data from all active processors.
    tod::Data data{};
    std::vector<pdbg_target*> chipList;
    util::pdbg::getActiveProcessorChips(chipList);
    for (const auto& chip : chipList)
    {
        tod::collectTodFaultData(chip, data);
    }

    // For each topology:
    //  - First, check if the MDMT chip is reporting a fault. If so, it is
    //    likely that any downstream step check faults are due to the fault in
    //    the MDMT.
    //  - If MDMT is not reporting a fault, look for any network path errors
    //    from the non-MDMT chips. In which case, we will want to call out all
    //    of the chips sourcing those step check errors (not the chips reporting
    //    them).
    //  - If no other errors found, callout any chips reporting internal step
    //    check faults.

    bool calloutsMade = false; // need to keep track for default case.

    for (const auto top : {tod::Topology::ACTIVE, tod::Topology::BACKUP})
    {
        auto mdmtFault      = data.getMdmtFault(top);
        auto internalFaults = data.getInteralFaults(top);
        auto networkFaults  = data.getNetworkFaults(top);

        if (nullptr != mdmtFault) // MDMT fault
        {
            calloutsMade = true;

            // Callout the TOD clock (guard).
            io_servData.calloutClock(callout::ClockType::TOD_CLOCK,
                                     callout::Priority::MED, true);

            // Callout the MDMT chip (no guard).
            io_servData.calloutTarget(mdmtFault, callout::Priority::MED, true);

            // Callout everything in between.
            // TODO: This isn't necessary for now because the clock callout is
            //       the backplane. However, we may need a procedure callout
            //       for future systems.
        }
        else if (!networkFaults.empty()) // network path faults
        {
            calloutsMade = true;

            // Callout all chips with network errors (guard).
            for (const auto& chip : networkFaults)
            {
                io_servData.calloutTarget(chip, callout::Priority::MED, true);
            }
        }
        else if (!internalFaults.empty()) // interal path faults
        {
            calloutsMade = true;

            // Callout all chips with internal errors (guard).
            for (const auto& chip : internalFaults)
            {
                io_servData.calloutTarget(chip, callout::Priority::MED, true);
            }
        }
    }

    // If no callouts are made, default to calling out the chip that reported
    // the original attention.
    if (!calloutsMade)
    {
        io_servData.calloutTarget(util::pdbg::getTrgt(i_chip),
                                  callout::Priority::MED, true);
    }
}

} // namespace P10

PLUGIN_DEFINE_NS(P10_10, P10, tod_step_check_fault);
PLUGIN_DEFINE_NS(P10_20, P10, tod_step_check_fault);

} // namespace analyzer
