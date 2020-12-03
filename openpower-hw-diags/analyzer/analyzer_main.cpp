#include <assert.h>
#include <libpdbg.h>
#include <unistd.h>

#include <hei_main.hpp>
#include <phosphor-logging/log.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace analyzer
{

//------------------------------------------------------------------------------

// Forward references for externally defined functions.

/**
 * @brief Will get the list of active chip and initialize the isolator.
 * @param o_chips The returned list of active chips.
 */
void initializeIsolator(std::vector<libhei::Chip>& o_chips);

/**
 * @brief Will create and submit a PEL using the given data.
 * @param i_rootCause A signature defining the attention root cause.
 * @param i_isoData   The data gathered during isolation (for FFDC).
 */
void createPel(const libhei::Signature& i_rootCause,
               const libhei::IsolationData& i_isoData);

//------------------------------------------------------------------------------

const char* __attn(libhei::AttentionType_t i_attnType)
{
    const char* str = "";
    switch (i_attnType)
    {
        case libhei::ATTN_TYPE_CHECKSTOP:
            str = "CHECKSTOP";
            break;
        case libhei::ATTN_TYPE_UNIT_CS:
            str = "UNIT_CS";
            break;
        case libhei::ATTN_TYPE_RECOVERABLE:
            str = "RECOVERABLE";
            break;
        case libhei::ATTN_TYPE_SP_ATTN:
            str = "SP_ATTN";
            break;
        case libhei::ATTN_TYPE_HOST_ATTN:
            str = "HOST_ATTN";
            break;
        default:
            trace::err("Unsupported attention type: %u", i_attnType);
            assert(0);
    }
    return str;
}

uint32_t __trgt(const libhei::Signature& i_sig)
{
    uint8_t type = util::pdbg::getTrgtType(i_sig.getChip());
    uint32_t pos = util::pdbg::getChipPos(i_sig.getChip());

    // Technically, the FapiPos attribute is 32-bit, but not likely to ever go
    // over 24-bit.

    return type << 24 | (pos & 0xffffff);
}

uint32_t __sig(const libhei::Signature& i_sig)
{
    return i_sig.getId() << 16 | i_sig.getInstance() << 8 | i_sig.getBit();
}

//------------------------------------------------------------------------------

// Takes a signature list that will be filtered and sorted. The first entry in
// the returned list will be the root cause. If the returned list is empty,
// analysis failed.
void __filterRootCause(std::vector<libhei::Signature>& io_list)
{
    // For debug, trace out the original list of signatures before filtering.
    for (const auto& sig : io_list)
    {
        trace::inf("Signature: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(sig.getChip()), __sig(sig),
                   __attn(sig.getAttnType()));
    }

    // Special and host attentions are not supported by this user application.
    auto newEndItr =
        std::remove_if(io_list.begin(), io_list.end(), [&](const auto& t) {
            return (libhei::ATTN_TYPE_SP_ATTN == t.getAttnType() ||
                    libhei::ATTN_TYPE_HOST_ATTN == t.getAttnType());
        });

    // Shrink the vector, if needed.
    io_list.resize(std::distance(io_list.begin(), newEndItr));

    // START WORKAROUND
    // TODO: Filtering should be determined by the RAS Data Files provided by
    //       the host firmware via the PNOR (similar to the Chip Data Files).
    //       Until that support is available, use a rudimentary filter that
    //       first looks for any recoverable attention, then any unit checkstop,
    //       and then any system checkstop. This is built on the premise that
    //       recoverable errors could be the root cause of an system checkstop
    //       attentions. Fortunately, we just need to sort the list by the
    //       greater attention type value.
    std::sort(io_list.begin(), io_list.end(),
              [&](const auto& a, const auto& b) {
                  return a.getAttnType() > b.getAttnType();
              });
    // END WORKAROUND
}

//------------------------------------------------------------------------------

bool __logError(const std::vector<libhei::Signature>& i_sigList,
                const libhei::IsolationData& i_isoData)
{
    bool attnFound = false;

    if (i_sigList.empty())
    {
        trace::inf("No active attentions found");
    }
    else
    {
        attnFound = true;

        // The root cause attention is the first in the filtered list.
        libhei::Signature root = i_sigList.front();

        trace::inf("Root cause attention: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(root.getChip()), root.toUint32(),
                   __attn(root.getAttnType()));

        // Create and commit a PEL.
        createPel(root, i_isoData);
    }

    return attnFound;
}

//------------------------------------------------------------------------------

bool analyzeHardware()
{
    bool attnFound = false;

    trace::inf(">>> enter analyzeHardware()");

    if (util::pdbg::queryHardwareAnalysisSupported())
    {
        // Initialize the isolator and get all of the chips to be analyzed.
        trace::inf("Initializing the isolator...");
        std::vector<libhei::Chip> chips;
        initializeIsolator(chips);

        // Isolate attentions.
        trace::inf("Isolating errors: # of chips=%u", chips.size());
        libhei::IsolationData isoData{};
        libhei::isolate(chips, isoData);

        // Filter signatures to determine root cause. We'll need to make a copy
        // of the list so that the original list is maintained for the log.
        std::vector<libhei::Signature> sigList{isoData.getSignatureList()};
        __filterRootCause(sigList);

        // Create and commit a log.
        attnFound = __logError(sigList, isoData);

        // All done, clean up the isolator.
        trace::inf("Uninitializing isolator...");
        libhei::uninitialize();
    }
    else
    {
        trace::err("Hardware error analysis is not supported on this system");
    }

    trace::inf("<<< exit analyzeHardware()");

    return attnFound;
}

} // namespace analyzer
