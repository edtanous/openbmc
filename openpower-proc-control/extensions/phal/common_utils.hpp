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
 *  @param[in] procTarget - Processor target to check if primary or not
 *
 *  @return True/False
 */
bool isPrimaryProc(struct pdbg_target* procTarget);

} // namespace phal
} // namespace openpower
