
#include <analyzer/plugins/plugin.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

namespace P10
{

/**
 * @brief Handles TOD step check fault attentions.
 */
void tod_step_check_fault(unsigned int, const libhei::Chip& i_chip,
                          ServiceData& io_servData)
{
    // TODO: The TOD step check fault analysis is complicated. It requires
    //       analysis of multiple status registers on all processors in the
    //       system. Until that support is available, call out the TOD clock and
    //       the processor reporting the attention.
    trace::err("Warning: TOD step check fault handling not fully supported");
    io_servData.calloutClock(callout::ClockType::TOD_CLOCK,
                             callout::Priority::MED, true);
    io_servData.calloutTarget(util::pdbg::getTrgt(i_chip),
                              callout::Priority::MED, false);
}

} // namespace P10

PLUGIN_DEFINE_NS(P10_10, P10, tod_step_check_fault);
PLUGIN_DEFINE_NS(P10_20, P10, tod_step_check_fault);

} // namespace analyzer
