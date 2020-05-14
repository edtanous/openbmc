#pragma once

#include <attn/attn_config.hpp>

namespace attn
{

/** @brief Attention handler return codes */
enum ReturnCodes
{
    RC_SUCCESS = 0,
    RC_NOT_HANDLED,
    RC_ANALYZER_ERROR,
    RC_CFAM_ERROR
};

/**
 * @brief The main attention handler logic
 *
 * Check each processor for active attentions of type SBE Vital (vital),
 * System Checkstop (checkstop) and Special Attention (special) and handle
 * each as follows:
 *
 * checkstop: Call hardware error analyzer
 * vital:     TBD
 * special:   Determine if the special attention is a Breakpoint (BP),
 *            Terminate Immediately (TI) or CoreCodeToSp (corecode). For each
 *            special attention type, do the following:
 *
 *            BP:          Notify Cronus
 *            TI:          Start host diagnostics mode systemd unit
 *            Corecode:    TBD
 *
 * @param i_config pointer to attention handler configuration object
 */
void attnHandler(Config* i_config);

} // namespace attn
