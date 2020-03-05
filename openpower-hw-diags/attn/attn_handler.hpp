#pragma once

namespace attn
{

/**
 * @brief The main attention handler logic
 *
 * Check each processor for active attentions of type SBE Vital (vital),
 * System Checkstop (checkstop) and Special Attention (special) and handle
 * each as follows:
 *
 * checkstop: TBD
 * vital:     TBD
 * special:   Determine if the special attention is a Breakpoint (BP),
 *            Terminate Immediately (TI) or CoreCodeToSp (corecode). For each
 *            special attention type, do the following:
 *
 *            BP:          Notify Cronus
 *            TI:          TBD
 *            Corecode:    TBD
 *            Recoverable: TBD
 *
 * @param i_breakpoints true = breakpoint special attn handling enabled
 */
void attnHandler(const bool i_breakpoints);

} // namespace attn
