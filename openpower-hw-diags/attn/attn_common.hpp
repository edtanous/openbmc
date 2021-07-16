#pragma once

namespace attn
{

/** @brief Attention handler return codes */
enum ReturnCodes
{
    RC_SUCCESS        = 0,
    RC_NOT_HANDLED    = 1,
    RC_ANALYZER_ERROR = 2,
    RC_CFAM_ERROR     = 3,
    RC_DBUS_ERROR     = 4
};

/** @brief Code seciton for error reporing */
enum class AttnSection
{
    reserved        = 0x0000,
    attnHandler     = 0x0100,
    tiHandler       = 0x0200,
    handlePhypTi    = 0x0300,
    handleHbTi      = 0x0400,
    addHbStatusRegs = 0x0500
};

/** @brief Attention handler error reason codes */
enum AttnCodes
{
    ATTN_NO_ERROR  = 0,
    ATTN_INFO_NULL = 1,
    ATTN_PDBG_CFAM = 2,
    ATTN_PDBG_SCOM = 3
};

enum class HostState
{
    Quiesce,
    Diagnostic,
    Crash
};

/**
 * @brief Transition the host state
 *
 * We will transition the host state by starting the appropriate dbus target.
 *
 * @param i_hostState the state to transition the host to
 */
void transitionHost(const HostState i_hostState);

/**
 * @brief Traces some regs for hostboot
 *
 * When we receive a Checkstop or special Attention Term Immediate,
 * hostboot wants some regs added to the error log. We will do this
 * by tracing them and then the traces will get added to the error
 * log later.
 *
 * @return nothing
 */
void addHbStatusRegs();

} // namespace attn
