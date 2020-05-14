#pragma once

#include <map>
#include <string>

namespace analyzer
{

/**
 * @brief Analyze and isolate hardware errors
 *
 * If any error conditions are found on the host isolate the hardware
 * component that caused the error(s).
 */

bool analyzeHardware(std::map<std::string, std::string>& i_errors);

} // namespace analyzer
