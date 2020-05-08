#pragma once

/**
 * @file hei_user_interface.hpp
 *
 * The method for actions like hardware register access will vary per user
 * application. Therefore, the user application must define all of the APIs
 * listed below.
 *
 *  1.  ReturnCode libhei::registerRead(const Chip& i_chip, void* o_buffer,
 *                      size_t& io_bufSize, uint64_t i_regType,
 *                      uint64_t i_address);
 *
 *  2.  ReturnCode libhei::registerWrite(const Chip& i_chip, void* i_buffer,
 *                            size_t& io_bufSize, uint64_t i_regType,
 *                            uint64_t i_address);
 *
 *  3. void libhei::hei_inf(...)
 *  4. void libhei::hei_err(...)
 *
 *  Example user application implementation of hei_inf(...) and hei_err(...)
 *
 *  void hei_inf(char* format, ...)
 *  {
 *      va_list args;
 *
 *      printf("I> ");
 *      va_start(args, format);
 *      vprintf(format, args);
 *      va_end(args);
 *      printf("\n");
 *  }
 *
 *  void hei_err(char* format, ...)
 *  {
 *      va_list args;
 *
 *      printf("E> ");
 *      va_start(args, format);
 *      vprintf(format, args);
 *      va_end(args);
 *      printf("\n");
 *  }
 */

#include <stdlib.h>

#include <hei_chip.hpp>

namespace libhei
{

/**
 * @brief Performs a hardware register read operation.
 *
 * @param i_chip     The target chip for the register access. It is provided to
 *                   the isolator by the user application via the isolator main
 *                   APIs.
 *
 * @param o_buffer   Allocated memory space for the returned contents of the
 *                   register.
 *
 * @param io_bufSize The input parameter is the maximum size of the allocated
 *                   o_buffer. The return value is the number of bytes actually
 *                   written to the buffer.
 *
 * @param i_regType  The user application may support different types of
 *                   registers. This value is provided to the isolator by the
 *                   user application via the Chip Data Files. The user
 *                   application is responsible for knowing what to do with this
 *                   parameter.
 *
 * @param i_address  The register address. The values is a 1, 2, 4, or 8 byte
 *                   address (right justified), which is provided to the
 *                   isolator by the user application via the Chip Data Files.
 *
 * @return false => register access was successful
 *         true  => hardware access failure
 *         Note that in the case of a failure, the user application is
 *         responsible for reporting why the register access failed.
 */
bool registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                  uint64_t i_regType, uint64_t i_address);

#ifndef __HEI_READ_ONLY

/**
 * @brief Performs a hardware register write operation.
 *
 * @param i_chip     The target chip for the register access. It is provided to
 *                   the isolator by the user application via the isolator main
 *                   APIs.
 *
 * @param i_buffer   Allocated memory space containing the register contents to
 *                   write to hardware.
 *
 * @param io_bufSize The input parameter is the number of byte from i_buffer to
 *                   write to the hardware register. The return value is the
 *                   actual number of bytes written to the hardware register.
 *
 * @param i_regType  The user application may support different types of
 *                   registers. This value is provided to the isolator by the
 *                   user application via the Chip Data Files. The user
 *                   application is responsible for knowing what to do with this
 *                   parameter.
 *
 * @param i_address  The register address. The values is a 1, 2, 4, or 8 byte
 *                   address (right justified), which is provided to the
 *                   isolator by the user application via the Chip Data Files.
 *
 * @return false => register access was successful
 *         true  => hardware access failure
 *         Note that in the case of a failure, the user application is
 *         responsible for reporting why the register access failed.
 */
bool registerWrite(const Chip& i_chip, void* i_buffer, size_t& io_bufSize,
                   uint64_t i_regType, uint64_t i_address);

// used by HEI_INF macro in this library
void hei_inf(char* format, ...); // implemented in user application

// used by HEI_ERR macro in this library
void hei_err(char* format, ...); // implemented in user application

#endif

} // end namespace libhei
