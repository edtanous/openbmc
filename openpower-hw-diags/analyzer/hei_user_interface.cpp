/**
 * @file These are the implementations of the user interfaces declared
 *       in hei_user_interface.hpp
 */

#include <assert.h>
#include <inttypes.h>
#include <libpdbg.h>
#include <stdarg.h>
#include <stdio.h>

#include <hei_user_interface.hpp>
#include <util/trace.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

bool registerRead(const Chip& i_chip, RegisterType_t i_regType,
                  uint64_t i_address, uint64_t& o_value)
{
    bool accessFailure = false;

    auto trgt = (pdbg_target*)(i_chip.getChip());

    switch (i_regType)
    {
        case REG_TYPE_SCOM:
        case REG_TYPE_ID_SCOM:
            // Read the 64-bit SCOM register.
            accessFailure = (0 != pib_read(trgt, i_address, &o_value));
            break;

        default:
            assert(0); // an unsupported register type
    }

    if (accessFailure)
    {
        trace::err("Register read failed: chip=%s type=0x%0" PRIx8
                   "addr=0x%0" PRIx64,
                   pdbg_target_path(trgt), i_regType, i_address);
        o_value = 0; // just in case
    }

    return accessFailure;
}

//------------------------------------------------------------------------------

// prints a single line to stdout
void hei_inf(char* format, ...)
{
    va_list args;
    va_start(args, format);
    trace::inf(format, args);
    va_end(args);
}

//------------------------------------------------------------------------------

// prints a single line to stderr
void hei_err(char* format, ...)
{
    va_list args;
    va_start(args, format);
    trace::err(format, args);
    va_end(args);
}

} // namespace libhei
