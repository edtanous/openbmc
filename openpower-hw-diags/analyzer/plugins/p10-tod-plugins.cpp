
#include <analyzer/plugins/plugin.hpp>
#include <hei_main.hpp>
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

/** Each topology can be configured as either primary or secondary. */
enum class Configuration
{
    PRIMARY,
    SECONDARY,
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

enum class Register
{
    TOD_ERROR           = 0x00040030,
    TOD_PSS_MSS_STATUS  = 0x00040008,
    TOD_PRI_PORT_0_CTRL = 0x00040001,
    TOD_PRI_PORT_1_CTRL = 0x00040002,
    TOD_SEC_PORT_0_CTRL = 0x00040003,
    TOD_SEC_PORT_1_CTRL = 0x00040004,
};

bool readRegister(pdbg_target* i_chip, Register i_addr,
                  libhei::BitStringBuffer& o_val)
{
    assert(64 == o_val.getBitLen());

    uint64_t scomValue;
    if (util::pdbg::getScom(i_chip, static_cast<uint64_t>(i_addr), scomValue))
    {
        trace::err("Register read failed: addr=0x%08x chip=%s",
                   static_cast<uint64_t>(i_addr), util::pdbg::getPath(i_chip));
        return true; // SCOM failed
    }

    o_val.setFieldRight(0, 64, scomValue);

    return false; // no failures
}

pdbg_target* getChipSourcingClock(pdbg_target* i_chipReportingError,
                                  unsigned int i_iohsPos)
{
    using namespace util::pdbg;

    pdbg_target* chipSourcingClock = nullptr;

    // Given the chip reporting the error and the IOHS position within that
    // chip, we must get
    //  - The associated IOHS target on this chip.
    //  - Next, the IOHS target on the other side of the bus.
    //  - Finally, the chip containing the IOHS target on the other side of the
    //    bus.

    auto iohsUnit = getChipUnit(i_chipReportingError, TYPE_IOHS, i_iohsPos);
    if (nullptr != iohsUnit)
    {
        auto clockSourceUnit =
            getConnectedTarget(iohsUnit, callout::BusType::SMP_BUS);
        if (nullptr != clockSourceUnit)
        {
            chipSourcingClock = getParentChip(clockSourceUnit);
        }
    }

    return chipSourcingClock;
}

/**
 * @brief Collects TOD fault data for each processor chip.
 */
void collectTodFaultData(pdbg_target* i_chip, Data& o_data)
{
    // TODO: We should use a register cache captured by the isolator so that
    //       this code is using the same values the isolator used.  However, at
    //       the moment the isolator does not have a register cache. Instead,
    //       we'll have to manually SCOM the registers we need.  Fortunately,
    //       for a checkstop attention the hardware should freeze and the
    //       values will never change. Unfortunately, we don't have that same
    //       guarantee for TIs, but at the time of this writing, all TOD errors
    //       will trigger a checkstop attention away. So the TI case is not as
    //       important.

    libhei::BitStringBuffer errorReg{64};
    if (readRegister(i_chip, Register::TOD_ERROR, errorReg))
    {
        return; // cannot continue on this chip
    }

    libhei::BitStringBuffer statusReg{64};
    if (readRegister(i_chip, Register::TOD_PSS_MSS_STATUS, statusReg))
    {
        return; // cannot continue on this chip
    }

    // Determine which topology is configured primary or secondary.
    std::map<Topology, Configuration> topConfig;

    if (0 == statusReg.getFieldRight(0, 3))
    {
        // TOD_PSS_MSS_STATUS[0:2] == 0b000 means active topology is primary.
        topConfig[Topology::ACTIVE] = Configuration::PRIMARY;
        topConfig[Topology::BACKUP] = Configuration::SECONDARY;
    }
    else
    {
        // TOD_PSS_MSS_STATUS[0:2] == 0b111 means active topology is secondary.
        topConfig[Topology::ACTIVE] = Configuration::SECONDARY;
        topConfig[Topology::BACKUP] = Configuration::PRIMARY;
    }

    for (const auto top : {Topology::ACTIVE, Topology::BACKUP})
    {
        // Bit positions in some registers are dependent on this topology's
        // configuration.
        bool isPriTop = (Configuration::PRIMARY == topConfig[top]);

        // Determine if this is the MDMT chip.
        bool isMasterTod    = statusReg.isBitSet(isPriTop ? 13 : 17);
        bool isMasterDrawer = statusReg.isBitSet(isPriTop ? 14 : 18);

        if (isMasterDrawer && isMasterTod)
        {
            // The master path selects are sourced from the oscilator reference
            // clocks. So, we'll need to determine which one was used at the
            // time of the failure.
            auto masterPathSelect =
                statusReg.getFieldRight(isPriTop ? 12 : 16, 1);

            // Determine if there is a step check fault for this path select.
            if (errorReg.isBitSet((0 == masterPathSelect) ? 14 : 15))
            {
                trace::inf(
                    "TOD MDMT fault found: top=%u config=%u path=%u chip=%s",
                    static_cast<unsigned int>(top),
                    static_cast<unsigned int>(topConfig[top]), masterPathSelect,
                    util::pdbg::getPath(i_chip));

                o_data.setMdmtFault(top, i_chip);
            }
        }
        else // not the MDMT on this topology
        {
            // The slave path selects are sourced from other processor chips.
            // So, we'll need to determine which one was used at the time of the
            // failure.
            auto slavePathSelect =
                statusReg.getFieldRight(isPriTop ? 15 : 19, 1);

            // Determine if there is a step check fault for this path select.
            if (errorReg.isBitSet((0 == slavePathSelect) ? 16 : 21))
            {
                // Get the IOHS unit position on this chip that is connected to
                // the clock source chip.
                auto addr = (0 == slavePathSelect)
                                ? (isPriTop ? Register::TOD_PRI_PORT_0_CTRL
                                            : Register::TOD_SEC_PORT_0_CTRL)
                                : (isPriTop ? Register::TOD_PRI_PORT_1_CTRL
                                            : Register::TOD_SEC_PORT_1_CTRL);

                libhei::BitStringBuffer portCtrl{64};
                if (readRegister(i_chip, addr, portCtrl))
                {
                    continue; // try the other topology
                }

                auto iohsPos           = portCtrl.getFieldRight(0, 3);
                auto chipSourcingClock = getChipSourcingClock(i_chip, iohsPos);

                if (nullptr != chipSourcingClock)
                {
                    trace::inf("TOD network fault found: top=%u config=%u "
                               "path=%u chip=%s iohs=%u clockSrc=%s",
                               static_cast<unsigned int>(top),
                               static_cast<unsigned int>(topConfig[top]),
                               slavePathSelect, util::pdbg::getPath(i_chip),
                               iohsPos, util::pdbg::getPath(chipSourcingClock));

                    o_data.setNetworkFault(top, chipSourcingClock);
                }
            }
        }

        // Check for any internal path errors in the active topology only.
        if (Topology::ACTIVE == top && errorReg.isBitSet(17))
        {
            trace::inf("TOD internal fault found: top=%u config=%u chip=%s",
                       static_cast<unsigned int>(top),
                       static_cast<unsigned int>(topConfig[top]),
                       util::pdbg::getPath(i_chip));

            o_data.setInternalFault(top, i_chip);
        }
    }
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
