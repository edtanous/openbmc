#include <attn/logging.hpp>

namespace attn
{

/** @brief Handle TI special attention */
void tiHandler()
{
    // trace message
    log<level::INFO>("TI handler");
}

} // namespace attn
