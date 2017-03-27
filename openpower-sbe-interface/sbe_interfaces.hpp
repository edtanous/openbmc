#pragma once

#include <stdint.h>

namespace openpower
{
namespace sbe
{
namespace scom
{

/**
 * @brief Read processor SCOM register.
 *
 * Throws an exception on error.
 *
 * @param[in] FIFO device path associated with the SBE.
 * @param[in] SCOM register address.
 * @return The register data
 */
uint64_t read(const char* devPath,
              uint64_t address);


/**
 * @brief Write processor SCOM register.
 *
 * Throws an exception on error.
 *
 * @param[in] FIFO device path associated with the SBE.
 * @param[in] SCOM register address.
 * @param[in] Data to be written into the register.
 */
void write(const char* devPath,
           uint64_t address,
           uint64_t data);

}//namespace scom
}//namespace sbe
}//namespace openpower
