#include <attn/attention.hpp>    // for Attention
#include <attn/attn_handler.hpp> // for RC_SUCCESS
#include <attn/attn_logging.hpp> // for trace

namespace attn
{

/** @brief Handle SBE vital attention */
int handleVital(Attention* i_attention)
{
    int rc = RC_SUCCESS;

    // trace message
    trace<level::INFO>("Vital handler");

    // sanity check
    if (nullptr == i_attention)
    {
        trace<level::INFO>("attention type is null");
        rc = RC_NOT_HANDLED;
    }

    return rc;
}

} // namespace attn
