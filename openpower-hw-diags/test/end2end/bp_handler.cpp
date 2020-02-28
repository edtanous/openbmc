#include <attn/logging.hpp>

#include <sstream>

namespace attn
{

/** @brief Breakpoint special attention handler */
void bpHandler()
{
    // trace message
    std::stringstream ss;
    ss << "breakpoint handler";
    log<level::INFO>(ss.str().c_str());
}

} // namespace attn
