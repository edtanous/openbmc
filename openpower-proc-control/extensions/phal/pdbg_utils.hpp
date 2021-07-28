#pragma once

#include <libipl.H>

extern "C"
{
#include <libpdbg.h>
}

namespace openpower
{
namespace phal
{

/**
 *  @brief  Read the input CFAM register
 *
 *  @param[in]  procTarget - Processor target to perform the operation on
 *  @param[in]  reg - The register address to read
 *  @param[out] val - The value read from the register
 *
 *  @return 0 on success, non-0 on failure
 */
uint32_t getCFAM(struct pdbg_target* procTarget, const uint32_t reg,
                 uint32_t& val);

/**
 *  @brief  Write the input CFAM register
 *
 *  @param[in]  procTarget - Processor target to perform the operation on
 *  @param[in]  reg - The register address to write
 *  @param[out] val - The value to write to the register
 *
 *  @return 0 on success, non-0 on failure
 */
uint32_t putCFAM(struct pdbg_target* procTarget, const uint32_t reg,
                 const uint32_t val);

/**
 *  @brief  Helper function to find FSI target needed for FSI operations
 *
 *  @param[in]  procTarget - Processor target to find the FSI target on
 *
 *  @return Valid pointer to FSI target on success, nullptr on failure
 */
pdbg_target* getFsiTarget(struct pdbg_target* procTarget);

/**
 *  @brief  Helper function to probe the processor target
 *
 *  The probe call only has to happen once per application start so ensure
 *  this function only probes once no matter how many times it's called.
 *
 *  @param[in]  procTarget - Processor target to probe
 *
 *  @return 0 on success, non-0 on failure
 */
uint32_t probeTarget(struct pdbg_target* procTarget);

/**
 * @brief Helper function to set PDBG_DTB
 *
 * PDBG_DTB environment variable set to CEC device tree path
 */
void setDevtreeEnv();

} // namespace phal
} // namespace openpower
