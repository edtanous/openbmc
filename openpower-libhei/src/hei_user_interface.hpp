#pragma once

/**
 * @file hei_user_interface.hpp
 *
 * The process for actions like hardware register access will vary per user
 * application. Therefore, the user application must define all of the APIs
 * declared in this header file.
 */

#include <stdlib.h>

#include <hei_chip.hpp>
#include <hei_types.hpp>

namespace libhei
{

/**
 * @brief Performs a hardware register read operation.
 *
 * @param i_chip     The target chip for the register access. It is provided to
 *                   the isolator by the user application via the isolator main
 *                   APIs.
 *
 * @param i_regType  The user application may support different types of
 *                   registers. This value is provided to the isolator by the
 *                   user application via the Chip Data Files. The user
 *                   application is responsible for knowing what to do with this
 *                   parameter.
 *
 * @param i_address  The register address, which is provided to the isolator by
 *                   the user application via the Chip Data Files. This is a 1,
 *                   2, 4, or 8 byte value (right justified) as defined by the
 *                   register type.
 *
 * @param o_value    The returned register value. This is a 1, 2, 4, or 8 byte
 *                   value (right justified) as defined by the register type.
 *
 * @return false => register access was successful
 *         true  => hardware access failure
 *         Note that in the case of a failure, the user application is
 *         responsible for reporting why the register access failed.
 */
bool registerRead(const Chip& i_chip, RegisterType_t i_regType,
                  uint64_t i_address, uint64_t& o_value);

#ifdef __HEI_ENABLE_HW_WRITE

/**
 * @brief Performs a hardware register write operation.
 *
 * @param i_chip     The target chip for the register access. It is provided to
 *                   the isolator by the user application via the isolator main
 *                   APIs.
 *
 * @param i_regType  The user application may support different types of
 *                   registers. This value is provided to the isolator by the
 *                   user application via the Chip Data Files. The user
 *                   application is responsible for knowing what to do with this
 *                   parameter.
 *
 * @param i_address  The register address, which is provided to the isolator by
 *                   the user application via the Chip Data Files. This is a 1,
 *                   2, 4, or 8 byte value (right justified) as defined by the
 *                   register type.
 *
 * @param i_value    The register value to write. This is a 1, 2, 4, or 8 byte
 *                   value (right justified) as defined by the register type.
 *
 * @return false => register access was successful
 *         true  => hardware access failure
 *         Note that in the case of a failure, the user application is
 *         responsible for reporting why the register access failed.
 */
bool registerWrite(const Chip& i_chip, RegisterType_t i_regType,
                   uint64_t i_address, uint64_t i_value);

#endif // __HEI_ENABLE_HW_WRITE

/**
 * @brief Prints an informational trace/log.
 *
 * This is similar to printing a single line of text to stdout. Example
 * implementation:
 *     void hei_inf(char* format, ...)
 *     {
 *         va_list args;
 *         va_start(args, format);
 *         vfprintf(stdout, format, args);
 *         va_end(args);
 *         fprintf(stdout, "\n");
 *     }
 *
 * @param i_format The string format and variatic arguments.
 */
void hei_inf(char* i_format, ...);

/**
 * @brief Prints an error trace/log.
 *
 * This is similar to printing a single line of text to stderr. Example
 * implementation:
 *     void hei_inf(char* format, ...)
 *     {
 *         va_list args;
 *         va_start(args, format);
 *         vfprintf(stderr, format, args);
 *         va_end(args);
 *         fprintf(stderr, "\n");
 *     }
 *
 * @param i_format The string format and variatic arguments.
 */
void hei_err(char* i_format, ...);

} // end namespace libhei
