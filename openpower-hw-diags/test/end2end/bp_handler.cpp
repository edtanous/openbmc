#include <attn/attn_logging.hpp>

namespace attn
{

/** @brief Breakpoint special attention handler */
void bpHandler()
{
    // trace message
    trace<level::INFO>("breakpoint handler");
}

} // namespace attn
