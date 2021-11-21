#pragma once

#include <attn/attn_dump.hpp>

namespace analyzer
{

enum class AnalysisType
{
    /**
     * Queries for the root cause of a system checkstop attention. An
     * unrecoverable PEL will be logged containing any necessary service actions
     * and the associated FFDC from analysis.
     */
    SYSTEM_CHECKSTOP,

    /**
     * Queries for any active recoverable or unit checkstop attentions that may
     * be attributed to a Terminate Immediate (TI) event. If any are found, an
     * predictive PEL will be logged containing any necessary service actions
     * and the associated FFDC from analysis.
     */
    TERMINATE_IMMEDIATE,

    /**
     * Queries for any active attentions. If any are found, an informational PEL
     * will be logged containing the FFDC from analysis (no service actions
     * applied).
     */
    MANUAL,
};

/**
 * @brief  Queries all chips in the host hardware for any active attentions.
 *         Then, it will perform any required RAS service actions based on the
 *         given analysis type.
 *
 * @param  i_type The type of analysis to perform. See enum above.
 *
 * @param  o_dump The returned dump data. This data is only set if the input
 *                value of i_type is SYSTEM_CHECKSTOP.
 *
 * @return The platform log ID (PLID) of the PEL generated during analysis. Will
 *         return zero if no PEL is generated.
 */
uint32_t analyzeHardware(AnalysisType i_type, attn::DumpParameters& o_dump);

/**
 * @brief Get error analyzer build information
 *
 * @return Pointer to build information
 */
const char* getBuildInfo();

} // namespace analyzer
