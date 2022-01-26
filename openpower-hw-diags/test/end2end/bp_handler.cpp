#include <util/trace.hpp>

namespace attn
{

/** @brief Breakpoint special attention handler */
void bpHandler()
{
    // trace message
    trace::inf("breakpoint handler");
}

} // namespace attn
