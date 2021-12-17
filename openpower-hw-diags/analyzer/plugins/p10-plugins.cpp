
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

} // namespace P10

PLUGIN_DEFINE_NS(P10_10, P10, pll_unlock);
PLUGIN_DEFINE_NS(P10_20, P10, pll_unlock);

} // namespace analyzer
