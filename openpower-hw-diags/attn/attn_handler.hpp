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

/** @brief Attention global status bits */
constexpr uint32_t SBE_ATTN       = 0x00000002;
constexpr uint32_t CHECKSTOP_ATTN = 0x40000000;
constexpr uint32_t SPECIAL_ATTN   = 0x20000000;

// Need to add defaultHbTiInfo with SRC BC801B99 (hex)

// Need to add defaultOpalTiInfo with SRC BB821410 (ascii)

constexpr uint8_t defaultPhypTiInfo[0x58] = {
    0x01, 0xa1, 0x02, 0xa8, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00,
    0x09, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x42, 0x37, 0x30, 0x30, 0x46, 0x46, 0x46,
    0x46, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/**
 * @brief The main attention handler logic
 *
 * Check each processor for active attentions of type SBE Vital (vital),
 * System Checkstop (checkstop) and Special Attention (special) and
 * handle each as follows:
 *
 * checkstop: Call hardware error analyzer
 * vital:     TBD
 * special:   Determine if the special attention is a Breakpoint (BP),
 *            Terminate Immediately (TI) or CoreCodeToSp (corecode). For
 * each special attention type, do the following:
 *
 *            BP:          Notify Cronus
 *            TI:          Start host diagnostics mode systemd unit
 *            Corecode:    TBD
 *
 * @param i_config pointer to attention handler configuration object
 */
void attnHandler(Config* i_config);

} // namespace attn
