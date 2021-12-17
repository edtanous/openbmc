
#include <analyzer/plugins/plugin.hpp>
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
void pll_unlock(unsigned int i_instance, const libhei::Chip&, ServiceData&)
{
    // TODO
    trace::inf("pll_unlock plugin: i_instance=%u", i_instance);
}

} // namespace P10

PLUGIN_DEFINE_NS(P10_10, P10, pll_unlock);
PLUGIN_DEFINE_NS(P10_20, P10, pll_unlock);

} // namespace analyzer
