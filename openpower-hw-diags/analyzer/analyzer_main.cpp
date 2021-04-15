#include <assert.h>
#include <libpdbg.h>
#include <unistd.h>

#include <analyzer/service_data.hpp>
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
 * @param i_isoData   The data gathered during isolation (for FFDC).
 * @param i_servData  Data regarding service actions gathered during analysis.
 */
void createPel(const libhei::IsolationData& i_isoData,
               const ServiceData& i_servData);

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

//------------------------------------------------------------------------------

bool __filterRootCause(const libhei::IsolationData& i_isoData,
                       libhei::Signature& o_signature)
{
    // We'll need to make a copy of the list so that the original list is
    // maintained for the log.
    std::vector<libhei::Signature> sigList{i_isoData.getSignatureList()};

    // For debug, trace out the original list of signatures before filtering.
    for (const auto& sig : sigList)
    {
        trace::inf("Signature: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(sig.getChip()), sig.toUint32(),
                   __attn(sig.getAttnType()));
    }

    // Special and host attentions are not supported by this user application.
    auto newEndItr =
        std::remove_if(sigList.begin(), sigList.end(), [&](const auto& t) {
            return (libhei::ATTN_TYPE_SP_ATTN == t.getAttnType() ||
                    libhei::ATTN_TYPE_HOST_ATTN == t.getAttnType());
        });

    // Shrink the vector, if needed.
    sigList.resize(std::distance(sigList.begin(), newEndItr));

    // START WORKAROUND
    // TODO: Filtering should be determined by the RAS Data Files provided by
    //       the host firmware via the PNOR (similar to the Chip Data Files).
    //       Until that support is available, use a rudimentary filter that
    //       first looks for any recoverable attention, then any unit checkstop,
    //       and then any system checkstop. This is built on the premise that
    //       recoverable errors could be the root cause of an system checkstop
    //       attentions. Fortunately, we just need to sort the list by the
    //       greater attention type value.
    std::sort(sigList.begin(), sigList.end(),
              [&](const auto& a, const auto& b) {
                  return a.getAttnType() > b.getAttnType();
              });
    // END WORKAROUND

    // Check if a root cause attention was found.
    if (!sigList.empty())
    {
        // The entry at the front of the list will be the root cause.
        o_signature = sigList.front();
        return true;
    }

    return false; // default, no active attentions found.
}

//------------------------------------------------------------------------------

bool __analyze(const libhei::IsolationData& i_isoData)
{
    bool attnFound = false;

    libhei::Signature rootCause{};
    attnFound = __filterRootCause(i_isoData, rootCause);

    if (!attnFound)
    {
        // NOTE: It is possible for TI handling that there will not be an active
        //       attention. In which case, we will not do anything and let the
        //       caller of this function determine if this is the expected
        //       behavior.
        trace::inf("No active attentions found");
    }
    else
    {
        trace::inf("Root cause attention: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(rootCause.getChip()),
                   rootCause.toUint32(), __attn(rootCause.getAttnType()));

        // TODO: Perform service actions based on the root cause. The default
        // callout if none other exist is level 2 support.
        ServiceData servData{rootCause};
        servData.addCallout(std::make_shared<ProcedureCallout>(
            ProcedureCallout::NEXTLVL, Callout::Priority::HIGH));

        // Create and commit a PEL.
        createPel(i_isoData, servData);
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

        // Analyze the isolation data and perform service actions if needed.
        attnFound = __analyze(isoData);

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

//------------------------------------------------------------------------------

/**
 * @brief Get error isolator build information
 *
 * @return Pointer to build information
 */
const char* getBuildInfo()
{
    return libhei::getBuildInfo();
}

} // namespace analyzer
