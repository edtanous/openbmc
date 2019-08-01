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
 * @brief Perform a hardware read operation.
 * @param i_chip This is a pointer to a user application object that represents
 *               the target chip. The isolator does not know anything about this
 *               object or how to use it. It is provide by the user application
 *               via the Isolator APIs. The user application is responsible for
 *               knowing what to do with this parameter.
 */
ReturnCode deviceRead( void * i_chip );

#ifndef __HEI_READ_ONLY

/**
 * @brief Perform a hardware write operation.
 * @param i_chip This is a pointer to a user application object that represents
 *               the target chip. The isolator does not know anything about this
 *               object or how to use it. It is provide by the user application
 *               via the Isolator APIs. The user application is responsible for
 *               knowing what to do with this parameter.
 */
ReturnCode deviceWrite( void * i_chip );

#endif

} // end namespace libhei

