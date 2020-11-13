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

} // namespace pdbg

} // namespace util
