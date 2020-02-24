#pragma once

namespace attn
{

/**
 * @brief Load the attention handler as a gpio monitor
 *
 * Request the attention gpio for monitoring and attach the attention handler
 * as the gpio event handler.
 *
 * @param i_breakpoints     enables breakpoint special attn handling
 *
 * @return 0 == success
 */
int attnDaemon(bool i_breakpoints);

} // namespace attn
