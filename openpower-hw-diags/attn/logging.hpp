#pragma once

namespace attn
{

/** @brief Logging level types */
enum level
{
    INFO
};

/**
 * @brief Log message of different types
 *
 * Log messages of different types (level) such as informational, debug,
 * errors etc.
 */
template <level L>
void log(const char* i_message);

} // namespace attn
