#include <attn/attn_logging.hpp>

namespace attn
{

/** @brief Handle TI special attention */
void tiHandler()
{
    // trace message
    trace<level::INFO>("TI handler");
}

} // namespace attn
