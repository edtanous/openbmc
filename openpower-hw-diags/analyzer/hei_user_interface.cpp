/**
 * @file These are the implementations of the user interfaces declared
 *       in hei_user_interface.hpp
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include <hei_user_interface.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

bool registerRead(const Chip& i_chip, void* o_buffer, size_t& io_bufSize,
                  uint64_t i_regType, uint64_t i_address)
{
    bool accessFailure = false;

    assert(nullptr != o_buffer);
    assert(0 != io_bufSize);

    // TODO need real register read code
    printf("registerRead not implemented\n");

    return accessFailure;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

bool registerWrite(const Chip& i_chip, void* i_buffer, size_t& io_bufSize,
                   uint64_t i_regType, uint64_t i_address)
{
    bool accessFailure = false;

    assert(nullptr != i_buffer);
    assert(0 != io_bufSize);

    // TODO need real register write code
    printf("registerWrite not implemented\n");

    return accessFailure;
}

#endif

//------------------------------------------------------------------------------

// prints a single line to stdout
void hei_inf(char* format, ...)
{
    va_list args;
    fprintf(stdout, "I> ");
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n");
}

//------------------------------------------------------------------------------

// prints a single line to stderr
void hei_err(char* format, ...)
{
    va_list args;
    fprintf(stderr, "E> ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

} // namespace libhei
