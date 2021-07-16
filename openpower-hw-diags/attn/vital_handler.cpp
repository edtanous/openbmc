#include <attn/attention.hpp>
#include <attn/attn_common.hpp>
#include <attn/attn_logging.hpp>
#include <sdbusplus/bus.hpp>

namespace attn
{

/**
 * @brief Handle SBE vital attention
 *
 * @param i_attention Attention object
 * @return 0 indicates that the vital attention was successfully handled
 *         1 indicates that the vital attention was NOT successfully handled
 */
int handleVital(Attention* i_attention)
{
    int rc = RC_SUCCESS; // assume vital handled

    trace<level::INFO>("vital handler started");

    // if vital handling enabled, handle vital attention
    if (false == (i_attention->getConfig()->getFlag(enVital)))
    {
        trace<level::INFO>("vital handling disabled");
        rc = RC_NOT_HANDLED;
    }
    else
    {
        // transition host state after analyses
        transitionHost(HostState::Quiesce);

        // generate pel
        eventVital();
    }

    return rc;
}

} // namespace attn
