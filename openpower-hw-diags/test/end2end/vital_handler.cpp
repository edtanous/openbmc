#include <attn/attention.hpp>   // for Attention
#include <attn/attn_common.hpp> // for RC_SUCCESS
#include <util/trace.hpp>

namespace attn
{

/** @brief Handle SBE vital attention */
int handleVital(Attention* i_attention)
{
    int rc = RC_SUCCESS;

    // trace message
    trace::inf("Vital handler");

    // sanity check
    if (nullptr == i_attention)
    {
        trace::inf("attention type is null");
        rc = RC_NOT_HANDLED;
    }

    return rc;
}

} // namespace attn
