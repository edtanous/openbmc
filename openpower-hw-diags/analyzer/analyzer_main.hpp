#pragma once

namespace analyzer
{

/**
 * @brief Analyze and isolate hardware errors
 *
 * If any error conditions are found on the host isolate the hardware
 * component that caused the error(s).
 */
void analyzeHardware();

} // namespace analyzer
