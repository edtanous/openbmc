//#include <attn/attn_logging.hpp>
#include <util/trace.hpp>

namespace attn
{

/** @brief Handle TI special attention */
void tiHandler()
{
    // trace message
    trace::inf("TI handler");
}

} // namespace attn
