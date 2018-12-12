#pragma once

#include <string>

namespace utility
{

/** @brief Set the value of a specified gpio
 *
 *  @param[in] gpioName - GPIO to set
 *  @param[in] activeLow - is pin active at low voltage
 *  @param[in] asserted - is pin in active state
 *
 *  @return bool - success of setting the GPIO
 */
bool gpioSetValue(const std::string& gpioName, bool activeLow, bool asserted)
{
    /* TODO */
    return true;
}

} // namespace utility
