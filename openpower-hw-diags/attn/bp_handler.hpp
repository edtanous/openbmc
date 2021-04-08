#pragma once

namespace attn
{

/**
 * @brief Breakpoint special attention handler
 *
 * Handler for special attention events due to a breakpoint condition.
 *
 * @return RC_NOT_HANDLED if error, else RC_SUCCESS
 */
int bpHandler();

} // namespace attn
