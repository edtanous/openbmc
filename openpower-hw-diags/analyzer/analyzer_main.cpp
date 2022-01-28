#include <assert.h>
#include <unistd.h>

#include <analyzer/analyzer_main.hpp>
#include <analyzer/ras-data/ras-data-parser.hpp>
#include <analyzer/service_data.hpp>
#include <attn/attn_dump.hpp>
#include <hei_main.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

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
 * @brief  Will get the list of active chip and initialize the isolator.
 * @param  i_type      The type of analysis to perform. See enum for details.
 * @param  i_isoData   The data gathered during isolation (for FFDC).
 * @param  o_rootCause The returned root cause signature.
 * @return True, if root cause has been found. False, otherwise.
 */
bool filterRootCause(AnalysisType i_type,
                     const libhei::IsolationData& i_isoData,
                     libhei::Signature& o_rootCause);

/**
 * @brief Will create and submit a PEL using the given data.
 * @param i_servData  Data regarding service actions gathered during analysis.
 * @return The platform log ID. Will return zero if no PEL is generated.
 */
uint32_t createPel(const ServiceData& i_servData);

//------------------------------------------------------------------------------

const char* __attn(libhei::AttentionType_t i_type)
{
    const char* str = "";
    switch (i_type)
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
            trace::err("Unsupported attention type: %u", i_type);
            assert(0);
    }
    return str;
}

//------------------------------------------------------------------------------

const char* __analysisType(AnalysisType i_type)
{
    const char* str = "";
    switch (i_type)
    {
        case AnalysisType::SYSTEM_CHECKSTOP:
            str = "SYSTEM_CHECKSTOP";
            break;
        case AnalysisType::TERMINATE_IMMEDIATE:
            str = "TERMINATE_IMMEDIATE";
            break;
        case AnalysisType::MANUAL:
            str = "MANUAL";
            break;
        default:
            trace::err("Unsupported analysis type: %u", i_type);
            assert(0);
    }
    return str;
}

//------------------------------------------------------------------------------

uint32_t analyzeHardware(AnalysisType i_type, attn::DumpParameters& o_dump)
{
    uint32_t o_plid = 0; // default, zero indicates PEL was not created

    if (!util::pdbg::queryHardwareAnalysisSupported())
    {
        trace::err("Hardware error analysis is not supported on this system");
        return o_plid;
    }

    trace::inf(">>> enter analyzeHardware(%s)", __analysisType(i_type));

    // Initialize the isolator and get all of the chips to be analyzed.
    trace::inf("Initializing the isolator...");
    std::vector<libhei::Chip> chips;
    initializeIsolator(chips);

    // Isolate attentions.
    trace::inf("Isolating errors: # of chips=%u", chips.size());
    libhei::IsolationData isoData{};
    libhei::isolate(chips, isoData);

    // For debug, trace out the original list of signatures before filtering.
    for (const auto& sig : isoData.getSignatureList())
    {
        trace::inf("Signature: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(sig.getChip()), sig.toUint32(),
                   __attn(sig.getAttnType()));
    }

    // Filter for root cause attention.
    libhei::Signature rootCause{};
    bool attnFound = filterRootCause(i_type, isoData, rootCause);

    // If a root cause attention was found, or if this was a system checkstop,
    // generate a PEL.
    if (attnFound || AnalysisType::SYSTEM_CHECKSTOP == i_type)
    {
        if (attnFound)
        {
            trace::inf("Root cause attention: %s 0x%0" PRIx32 " %s",
                       util::pdbg::getPath(rootCause.getChip()),
                       rootCause.toUint32(), __attn(rootCause.getAttnType()));
        }
        else
        {
            // This is bad. Analysis should have found a root cause attention
            // for a system checkstop. Issues could range from code bugs to SCOM
            // errors. Regardless, generate a PEL with FFDC to assist with
            // debug.
            trace::err("System checkstop with no root cause attention");
            rootCause = libhei::Signature{}; // just in case
        }

        // Start building the service data.
        ServiceData servData{rootCause, i_type, isoData};

        // Apply any service actions, if needed. Note that there are no
        // resolutions for manual analysis.
        if (AnalysisType::MANUAL != i_type)
        {
            if (attnFound)
            {
                // Resolve the root cause attention.
                RasDataParser rasData{};
                rasData.getResolution(rootCause)->resolve(servData);
            }
            else
            {
                // Analysis failed so apply the Level 2 Support resolution.
                servData.calloutProcedure(callout::Procedure::NEXTLVL,
                                          callout::Priority::HIGH);
            }
        }

        // Create and commit a PEL.
        o_plid = createPel(servData);

        if (0 == o_plid)
        {
            trace::err("Failed to create PEL");
        }
        else
        {
            trace::inf("PEL created: PLID=0x%0" PRIx32, o_plid);

            // Gather/return information needed for dump. A hardware dump will
            // always be used for system checkstop attenions. Software dumps
            // will be reserved for MP-IPLs during TI analysis.
            // TODO: Need ID from root cause. At the moment, HUID does not exist
            //       in devtree. Will need a better ID definition.
            o_dump.unitId   = 0;
            o_dump.dumpType = attn::DumpType::Hardware;
        }
    }
    else
    {
        // It is possible for TI handling, or manually initiated analysis via
        // the command line, that there will not be an active attention. In
        // which case, we will do nothing and let the caller of this function
        // determine if this is the expected behavior.
        trace::inf("No active attentions found");
    }

    // All done, clean up the isolator.
    trace::inf("Uninitializing isolator...");
    libhei::uninitialize();

    trace::inf("<<< exit analyzeHardware()");

    return o_plid;
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
