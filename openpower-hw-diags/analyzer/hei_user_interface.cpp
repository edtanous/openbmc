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

const char* __regType(RegisterType_t i_regType)
{
    const char* str = "";
    switch (i_regType)
    {
        case REG_TYPE_SCOM:
            str = "SCOM";
            break;
        case REG_TYPE_ID_SCOM:
            str = "ID_SCOM";
            break;
        default:
            trace::err("Unsupported register type: i_regType=0x%02x",
                       i_regType);
            assert(0);
    }
    return str;
}

//------------------------------------------------------------------------------

bool __readProc(pdbg_target* i_procTrgt, RegisterType_t i_regType,
                uint64_t i_address, uint64_t& o_value)
{
    bool accessFailure = false;

    // The processor PIB target is required for SCOM access.
    char path[16];
    sprintf(path, "/proc%d/pib", pdbg_target_index(i_procTrgt));
    pdbg_target* scomTrgt = pdbg_target_from_path(nullptr, path);
    assert(nullptr != scomTrgt);

    switch (i_regType)
    {
        case REG_TYPE_SCOM:
        case REG_TYPE_ID_SCOM:
            // Read the 64-bit SCOM register.
            accessFailure = (0 != pib_read(scomTrgt, i_address, &o_value));
            break;

        default:
            trace::err("Unsupported register type: trgt=%s regType=0x%02x "
                       "addr=0x%0" PRIx64,
                       pdbg_target_path(i_procTrgt), i_regType, i_address);
            assert(0); // an unsupported register type
    }

    return accessFailure;
}

//------------------------------------------------------------------------------

bool __readOcmb(pdbg_target* i_obmcTrgt, RegisterType_t i_regType,
                uint64_t i_address, uint64_t& o_value)
{
    bool accessFailure = false;

    /* TODO: ocmb_getscom() currently does not exist upstream.
    // The OCMB target is used for SCOM access.
    pdbg_target* scomTrgt = i_obmcTrgt;

    switch (i_regType)
    {
        case REG_TYPE_SCOM:
        case REG_TYPE_ID_SCOM:
            // Read the 64-bit SCOM register.
            accessFailure = (0 != ocmb_getscom(scomTrgt, i_address, &o_value));
            break;

        default:
            trace::err("Unsupported register type: trgt=%s regType=0x%02x "
                       "addr=0x%0" PRIx64,
                       pdbg_target_path(i_obmcTrgt), i_regType, i_address);
            assert(0);
    }
    */

    return accessFailure;
}

//------------------------------------------------------------------------------

bool registerRead(const Chip& i_chip, RegisterType_t i_regType,
                  uint64_t i_address, uint64_t& o_value)
{
    bool accessFailure = false;

    auto trgt = (pdbg_target*)(i_chip.getChip());

    uint8_t trgtType = 0;
    pdbg_target_get_attribute(trgt, "ATTR_TYPE", 1, 1, &trgtType);

    switch (trgtType)
    {
        case 0x05: // PROC
            accessFailure = __readProc(trgt, i_regType, i_address, o_value);
            break;

        case 0x4b: // OCMB_CHIP
            accessFailure = __readOcmb(trgt, i_regType, i_address, o_value);
            break;

        default:
            trace::err("Unsupported target type: trgt=%s trgtType=0x%02x",
                       pdbg_target_path(trgt), trgtType);
            assert(0);
    }

    if (accessFailure)
    {
        trace::err("%s failure: trgt=%s addr=0x%0" PRIx64, __regType(i_regType),
                   pdbg_target_path(trgt), i_address);
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
