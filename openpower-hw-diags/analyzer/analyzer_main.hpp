#pragma once

namespace analyzer
{

/**
 * @brief  Queries the host hardware for all attentions reported by each active
 *         chip. Then it performs all approriate RAS actions based on the active
 *         attentions.
 *
 * @return True if an active attenion was successfully analyzed, false
 *         otherwise.
 *         For system checkstop handling:
 *            If analysis fails, there likely is a defect in the design because
 *            an active attention is required to trigger the interrupt.
 *         For TI handling:
 *            It is possible that a recoverable attention could cause a TI,
 *            however, it is not required. Therefore, it is expected that
 *            analysis could fail to find an attention and it should not be
 *            treated as a defect.
 */
bool analyzeHardware();

/**
 * @brief Get error analyzer build information
 *
 * @return Pointer to build information
 */
const char* getBuildInfo();

} // namespace analyzer
