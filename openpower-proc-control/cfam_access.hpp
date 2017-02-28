#pragma once

#include <memory>
#include "targeting.hpp"

namespace openpower
{
namespace cfam
{
namespace access
{

using cfam_address_t = uint16_t;
using cfam_data_t = uint32_t;
using cfam_mask_t = uint32_t;

/**
 * @brief Writes a CFAM (Common FRU Access Macro) register in a P9.
 *
 * Throws an exception on error.
 *
 * @param[in] target - The Target to perform the operation on
 * @param[in] address - The register address to write to
 * @param[in] data - The data to write
 */
void writeReg(const std::unique_ptr<openpower::targeting::Target>& target,
              cfam_address_t address,
              cfam_data_t data);


/**
 * @brief Reads a CFAM (Common FRU Access Macro) register in a P9.
 *
 * Throws an exception on error.
 *
 * @param[in] target - The Target to perform the operation on
 * @param[in] address - The register address to read
 * @return - The register data
 */
cfam_data_t readReg(
    const std::unique_ptr<openpower::targeting::Target>& target,
    cfam_address_t address);


/**
 * @brief Writes a CFAM (Common FRU Access Macro) register in a P9
 *        using a mask to specify the bits the modify.
 *
 * Only bits that are set in the mask parameter will be modified.
 *
 * Throws an exception on error.
 *
 * @param[in] target - The Target to perform the operation on
 * @param[in] address - The register address to write to
 * @param[in] data - The data to write
 * @param[in] mask - The mask
 */
void writeRegWithMask(
    const std::unique_ptr<openpower::targeting::Target>& target,
    cfam_address_t address,
    cfam_data_t data,
    cfam_mask_t mask);
}
}
}
