#pragma once

#include <libpdbg.h>

#include <hei_main.hpp>

namespace util
{

namespace pdbg
{

/** @return The target associated with the given chip. */
pdbg_target* getTrgt(const libhei::Chip& i_chip);

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
 * @brief Returns the list of all active chips in the system.
 * @param o_chips The returned list of chips.
 */
void getActiveChips(std::vector<libhei::Chip>& o_chips);

} // namespace pdbg

} // namespace util
