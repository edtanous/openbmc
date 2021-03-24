#pragma once

#include <libpdbg.h>

#include <string>
#include <vector>

// Forward reference to avoid pulling the libhei library into everything that
// includes this header.
namespace libhei
{
class Chip;
}

namespace util
{

namespace pdbg
{

/** Chip target types. */
enum TargetType_t : uint8_t
{
    TYPE_PROC = 0x05,
    TYPE_OCMB = 0x4b,
};

/** @return The target associated with the given chip. */
pdbg_target* getTrgt(const libhei::Chip& i_chip);

/** @return The target associated with the given devtree path. */
pdbg_target* getTrgt(const std::string& i_path);

/** @return A string representing the given target's devtree path. */
const char* getPath(pdbg_target* i_trgt);

/** @return A string representing the given chip's devtree path. */
const char* getPath(const libhei::Chip& i_chip);

/** @return The absolute position of the given target. */
uint32_t getChipPos(pdbg_target* i_trgt);

/** @return The absolute position of the given chip. */
uint32_t getChipPos(const libhei::Chip& i_chip);

/** @return The target type of the given target. */
uint8_t getTrgtType(pdbg_target* i_trgt);

/** @return The target type of the given chip. */
uint8_t getTrgtType(const libhei::Chip& i_chip);

/**
 * @return The pib target associated with the given proc target.
 * @note   Will assert the given target is a proc target.
 * @note   Will assert the returned pib target it not nullptr.
 */
pdbg_target* getPibTrgt(pdbg_target* i_procTrgt);

/**
 * @return The fsi target associated with the given proc target.
 * @note   Will assert the given target is a proc target.
 * @note   Will assert the returned fsi target it not nullptr.
 */
pdbg_target* getFsiTrgt(pdbg_target* i_procTrgt);

/**
 * @brief  Reads a CFAM FSI register.
 * @param  i_trgt Given target.
 * @param  i_addr Given address.
 * @param  o_val  The returned value of the register.
 * @return 0 if successful, non-0 otherwise.
 * @note   Will assert the given target is a proc target.
 */
int getCfam(pdbg_target* i_trgt, uint32_t i_addr, uint32_t& o_val);

/**
 * @brief Returns the list of all active chips in the system.
 * @param o_chips The returned list of chips.
 */
void getActiveChips(std::vector<libhei::Chip>& o_chips);

/**
 * @return True, if hardware analysis is supported on this system. False,
 *         otherwise.
 * @note   Support for hardware analysis from the BMC started with P10 systems
 *         and is not supported on any older chip generations.
 */
bool queryHardwareAnalysisSupported();

/**
 * @return A string containing the FRU location code of the given chip. An empty
 *         string indicates the target was null or the attribute does not exist
 *         for this target.
 * @note   This function requires PHAL APIs that are only available in certain
 *         environments. If they do not exist the devtree path of the target is
 *         returned.
 */
std::string getLocationCode(pdbg_target* trgt);

/**
 * @return A string containing the physical device path (entity path) of the
 *         given chip. An empty string indicates the target was null or the
 *         attribute does not exist for this target.
 * @note   This function requires PHAL APIs that are only available in certain
 *         environments. If they do not exist the devtree path of the target is
 *         returned.
 */
std::string getPhysDevPath(pdbg_target* trgt);

} // namespace pdbg

} // namespace util
