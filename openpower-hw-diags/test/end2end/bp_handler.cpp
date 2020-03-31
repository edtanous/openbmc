#include <attn/logging.hpp>

namespace attn
{

/** @brief Breakpoint special attention handler */
void bpHandler()
{
    // trace message
    log<level::INFO>("breakpoint handler");
}

} // namespace attn
