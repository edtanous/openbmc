#include <attn/logging.hpp>

#include <sstream>

namespace attn
{

/** @brief Handle TI special attention */
void tiHandler()
{
    // trace message
    std::stringstream ss;
    ss << "TI handler" << std::endl;
    log<level::INFO>(ss.str().c_str());
}

} // namespace attn
