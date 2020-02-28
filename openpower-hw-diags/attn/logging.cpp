#include <logging.hpp>
#include <phosphor-logging/log.hpp>

namespace attn
{

/** @brief Log message of type INFO using phosphor logging */
template <>
void log<INFO>(const char* i_message)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(i_message);
}

} // namespace attn
