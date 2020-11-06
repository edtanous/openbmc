#pragma once

#include <hei_main.hpp>

namespace util
{

namespace pdbg
{

/**
 * @param  A chip.
 * @return A string representing the chip's devtree path.
 */
const char* getPath(const libhei::Chip& i_chip);

} // namespace pdbg

} // namespace util
