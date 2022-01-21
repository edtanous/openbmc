
#include <analyzer/plugins/plugin.hpp>
#include <hei_util.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

namespace P10
{

/**
 * @brief Adds all clocks/chips reporting PLL unlock attentions to the callout
 *        list.
 *
 *  Processors are always called out at medium priority and never guarded. If
 *  more than one processor is reporting a PLL unlock attention on the same
 *  clock, the clock is called out with high priority. Otherwise, the clock
 *  callout priority is medium.
 */
void pll_unlock(unsigned int i_instance, const libhei::Chip&,
                ServiceData& io_servData)
{
    auto nodeId = libhei::hash<libhei::NodeId_t>("PLL_UNLOCK");

    auto sigList = io_servData.getIsolationData().getSignatureList();

    // The PLL list is initially the same size of the signature list.
    std::vector<libhei::Signature> pllList{sigList.size()};

    // Copy all signatures that match the node ID and bit position. Note that
    // in this case the bit position is the same as the plugin instance.
    auto itr = std::copy_if(sigList.begin(), sigList.end(), pllList.begin(),
                            [&nodeId, &i_instance](const auto& s) {
                                return (nodeId == s.getId() &&
                                        i_instance == s.getBit());
                            });

    // Shrink the size of the PLL list if necessary.
    pllList.resize(std::distance(pllList.begin(), itr));

    // The clock callout priority is dependent on the number of chips with PLL
    // unlock attentions.
    auto clockPriority =
        (1 < pllList.size()) ? callout::Priority::HIGH : callout::Priority::MED;

    // Callout the clock.
    auto clockCallout = (0 == i_instance) ? callout::ClockType::OSC_REF_CLOCK_0
                                          : callout::ClockType::OSC_REF_CLOCK_1;
    io_servData.calloutClock(clockCallout, clockPriority, true);

    // Callout the processors connected to this clock that are reporting PLL
    // unlock attentions. Always a medium callout and no guarding.
    for (const auto& sig : pllList)
    {
        io_servData.calloutTarget(util::pdbg::getTrgt(sig.getChip()),
                                  callout::Priority::MED, false);
    }
}

/**
 * @brief Queries for an LPC timeout. If present, will callout all appropriate
 *        hardware.
 */
void lpc_timeout(unsigned int, const libhei::Chip& i_chip,
                 ServiceData& io_servData)
{
    auto target = util::pdbg::getTrgt(i_chip);
    auto path   = util::pdbg::getPath(target);

    if (util::pdbg::queryLpcTimeout(target))
    {
        trace::inf("LPC timeout detected on %s", path);

        // Callout the PNOR.
        io_servData.calloutPart(callout::PartType::PNOR,
                                callout::Priority::MED);

        // Callout the associated clock, no guard.
        auto chipPos = util::pdbg::getChipPos(target);
        if (0 == chipPos)
        {
            // Clock 0 is hardwired to proc 0.
            io_servData.calloutClock(callout::ClockType::OSC_REF_CLOCK_0,
                                     callout::Priority::MED, false);
        }
        else if (1 == chipPos)
        {
            // Clock 1 is hardwired to proc 1.
            io_servData.calloutClock(callout::ClockType::OSC_REF_CLOCK_1,
                                     callout::Priority::MED, false);
        }
        else
        {
            trace::err("LPC timeout on unexpected processor: %s", path);
        }

        // Callout the processor, no guard.
        io_servData.calloutTarget(target, callout::Priority::MED, false);
    }
    else
    {
        trace::inf("No LPC timeout detected on %s", path);

        io_servData.calloutProcedure(callout::Procedure::NEXTLVL,
                                     callout::Priority::HIGH);
    }
}

/**
 * @brief Handles TOD step check fault attentions.
 */
void tod_step_check_fault(unsigned int, const libhei::Chip&, ServiceData&)
{
    // TODO
}

} // namespace P10

PLUGIN_DEFINE_NS(P10_10, P10, pll_unlock);
PLUGIN_DEFINE_NS(P10_20, P10, pll_unlock);

PLUGIN_DEFINE_NS(P10_10, P10, lpc_timeout);
PLUGIN_DEFINE_NS(P10_20, P10, lpc_timeout);

PLUGIN_DEFINE_NS(P10_10, P10, tod_step_check_fault);
PLUGIN_DEFINE_NS(P10_20, P10, tod_step_check_fault);

} // namespace analyzer
