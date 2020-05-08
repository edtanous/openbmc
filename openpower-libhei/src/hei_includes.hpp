#pragma once

/**
 * @file  hei_includes.hpp
 * @brief The purpose of this file is to include common headers that will be
 *        used throughout this library.
 */

// Standard library includes
#include <stdint.h>
#include <stdlib.h>

#include <map>
#include <vector>

// The user application must define "hei_user_defines.hpp" with the following
// macros:
//
//   Tracing (inputs same as printf() from <cstdio>):
//      HEI_INF(...) // Generic informational trace
//      HEI_ERR(...) // Error path trace
//
//  Assertion (at a minimum should work like assert() from <cassert>):
//      HEI_ASSERT(expression)
//

// Internal includes
#include <hei_chip.hpp>
#include <hei_types.hpp>
