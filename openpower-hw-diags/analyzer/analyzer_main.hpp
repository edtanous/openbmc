#pragma once

#include <attn/attn_dump.hpp>

namespace analyzer
{

/**
 * @brief  Queries the host hardware for all attentions reported by each active
 *         chip. Then it performs all approriate RAS actions based on the active
 *         attentions.
 *
 * @param[out] o_dumpParameters Dump request parameters
 * @return The platform log ID (PLID) of the PEL generated during analysis. Will
 *         return zero if no PEL is generated.
 */
uint32_t analyzeHardware(attn::DumpParameters& o_dumpParameters);

/**
 * @brief Get error analyzer build information
 *
 * @return Pointer to build information
 */
const char* getBuildInfo();

} // namespace analyzer
