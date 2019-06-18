
#pragma once

// The user application must define this header file with the following macros:
//
//   Tracing (inputs same as printf() from <cstdio>):
//      HEI_INF( ... ) // Generic informational trace
//      HEI_ERR( ... ) // Error path trace
//
//  Assertion (at a minimum should work like assert() from <cassert>):
//      HEI_ASSERT( expression )
//
#include <hei_user_defines.hpp>

namespace libhei
{

/**
 *  @brief Perform a hardware read operation.
 */
void deviceRead();

#ifndef __HEI_READ_ONLY

/**
 *  @brief Perform a hardware write operation.
 */
void deviceWrite();

#endif

} // end namespace libhei

