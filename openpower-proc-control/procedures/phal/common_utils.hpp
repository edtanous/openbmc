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
 * @brief This function will initialize required phal
 *        libraries.
 * Throws an exception on error.
 *
 * @param[in] mode - IPL mode, default IPL_AUTOBOOT
 *
 */
void phal_init(enum ipl_mode mode = IPL_AUTOBOOT);

/**
 *  @brief  Check if primary processor or not
 *
 *  @param[in] procTarget - Target to check if primary or not
 *
 *  @return True/False
 */
bool isPrimaryProc(struct pdbg_target* procTarget);

/**
 *  @brief  Read the input CFAM register
 *
 *  @param[in]  procTarget - The Target to perform the operation on
 *  @param[in]  reg - The register address to read
 *  @param[out] val - The value read from the register
 *
 *  @return 0 on success, non-0 on failure
 */
uint32_t getCFAM(struct pdbg_target* procTarget, const uint32_t reg,
                 uint32_t& val);

} // namespace phal
} // namespace openpower
