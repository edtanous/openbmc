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

namespace libhei
{

//------------------------------------------------------------------------------

bool registerRead(const Chip& i_chip, RegisterType_t i_regType,
                  uint64_t i_address, uint64_t& o_value)
{
    bool accessFailure = false;

    switch (i_regType)
    {
        case REG_TYPE_SCOM:
        case REG_TYPE_ID_SCOM:
            // Read the 64-bit SCOM register.
            accessFailure = (0 != pib_read((pdbg_target*)i_chip.getChip(),
                                           i_address, &o_value));
            break;

        default:
            assert(0); // an unsupported register type
    }

    if (accessFailure)
    {
        printf("Register read failed: chip=%p type=0x%0" PRIx8
               "addr=0x%0" PRIx64 "\n",
               i_chip.getChip(), i_regType, i_address);
        o_value = 0; // just in case
    }

    return accessFailure;
}

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
