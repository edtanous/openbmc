#include <assert.h>

#include <analyzer_main.hpp>
#include <hei_main.hpp>
#include <hei_util.hpp>
#include <util/pdbg.hpp>

#include <algorithm>
#include <limits>
#include <string>

namespace analyzer
{

//------------------------------------------------------------------------------

bool __findRcsOscError(const std::vector<libhei::Signature>& i_list,
                       libhei::Signature& o_rootCause)
{
    // TODO: Consider returning all of them instead of one as root cause.
    auto itr = std::find_if(i_list.begin(), i_list.end(), [&](const auto& t) {
        return (libhei::hash<libhei::NodeId_t>("TP_LOCAL_FIR") == t.getId() &&
                (42 == t.getBit() || 43 == t.getBit()));
    });

    if (i_list.end() != itr)
    {
        o_rootCause = *itr;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

bool __findPllUnlock(const std::vector<libhei::Signature>& i_list,
                     libhei::Signature& o_rootCause)
{
    // TODO: Consider returning all of them instead of one as root cause.
    auto itr = std::find_if(i_list.begin(), i_list.end(), [&](const auto& t) {
        return (libhei::hash<libhei::NodeId_t>("PLL_UNLOCK") == t.getId() &&
                (0 == t.getBit() || 1 == t.getBit()));
    });

    if (i_list.end() != itr)
    {
        o_rootCause = *itr;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

bool __findMemoryChannelFailure(const std::vector<libhei::Signature>& i_list,
                                libhei::Signature& o_rootCause)
{
    using namespace util::pdbg;

    using func  = libhei::NodeId_t (*)(const std::string& i_str);
    func __hash = libhei::hash<libhei::NodeId_t>;

    static const auto mc_dstl_fir       = __hash("MC_DSTL_FIR");
    static const auto mc_ustl_fir       = __hash("MC_USTL_FIR");
    static const auto mc_omi_dl_err_rpt = __hash("MC_OMI_DL_ERR_RPT");

    for (const auto s : i_list)
    {
        const auto targetType = getTrgtType(getTrgt(s.getChip()));
        const auto id         = s.getId();
        const auto bit        = s.getBit();
        const auto attnType   = s.getAttnType();

        // Look for any unit checkstop attentions from OCMBs.
        if (TYPE_OCMB == targetType)
        {
            // Any unit checkstop attentions will trigger a channel failure.
            if (libhei::ATTN_TYPE_UNIT_CS == attnType)
            {
                o_rootCause = s;
                return true;
            }
        }
        // Look for channel failure attentions on processors.
        else if (TYPE_PROC == targetType)
        {
            // TODO: All of these channel failure bits are configurable.
            //       Eventually, we will need some mechanism to check that
            //       config registers for a more accurate analysis. For now,
            //       simply check for all bits that could potentially be
            //       configured to channel failure.

            // Any unit checkstop bit in the MC_DSTL_FIR or MC_USTL_FIR could
            // be a channel failure.
            if (libhei::ATTN_TYPE_UNIT_CS == attnType)
            {
                // Ignore bits MC_DSTL_FIR[0:7] because they simply indicate
                // attentions occurred on the attached OCMBs.
                if ((mc_dstl_fir == id && 8 <= bit) || (mc_ustl_fir == id))
                {
                    o_rootCause = s;
                    return true;
                }
            }

            // All bits in MC_OMI_DL_ERR_RPT eventually feed into
            // MC_OMI_DL_FIR[0,20] which are configurable to channel failure.
            if (mc_omi_dl_err_rpt == id)
            {
                o_rootCause = s;
                return true;
            }
        }
    }

    return false; // default, nothing found
}

//------------------------------------------------------------------------------

// Will query if a signature is a potential system checkstop root cause.
// attention. Note that this function excludes memory channel failure attentions
// and core unit checkstop attentions.
bool __findCsRootCause(const libhei::Signature& i_signature)
{
    using namespace util::pdbg;

    using func  = libhei::NodeId_t (*)(const std::string& i_str);
    func __hash = libhei::hash<libhei::NodeId_t>;

    // PROC registers
    static const auto eq_core_fir      = __hash("EQ_CORE_FIR");
    static const auto eq_l2_fir        = __hash("EQ_L2_FIR");
    static const auto eq_l3_fir        = __hash("EQ_L3_FIR");
    static const auto eq_ncu_fir       = __hash("EQ_NCU_FIR");
    static const auto iohs_dlp_fir_oc  = __hash("IOHS_DLP_FIR_OC");
    static const auto iohs_dlp_fir_smp = __hash("IOHS_DLP_FIR_SMP");
    static const auto nx_cq_fir        = __hash("NX_CQ_FIR");
    static const auto nx_dma_eng_fir   = __hash("NX_DMA_ENG_FIR");
    static const auto pau_fir_0        = __hash("PAU_FIR_0");
    static const auto pau_fir_1        = __hash("PAU_FIR_1");
    static const auto pau_fir_2        = __hash("PAU_FIR_2");
    static const auto pau_ptl_fir      = __hash("PAU_PTL_FIR");

    // OCMB registers
    static const auto rdffir = __hash("RDFFIR");

    const auto targetType = getTrgtType(getTrgt(i_signature.getChip()));
    const auto id         = i_signature.getId();
    const auto bit        = i_signature.getBit();

    if (TYPE_PROC == targetType)
    {
        if (eq_core_fir == id &&
            (3 == bit || 5 == bit || 8 == bit || 12 == bit || 22 == bit ||
             25 == bit || 32 == bit || 36 == bit || 38 == bit || 46 == bit ||
             47 == bit))
        {
            return true;
        }

        if (eq_l2_fir == id &&
            (1 == bit || 12 == bit || 13 == bit || 17 == bit || 18 == bit ||
             20 == bit || 27 == bit))
        {
            return true;
        }

        if (eq_l3_fir == id &&
            (2 == bit || 5 == bit || 8 == bit || 11 == bit || 17 == bit))
        {
            return true;
        }

        if (eq_ncu_fir == id && (3 == bit || 4 == bit || 5 == bit || 7 == bit ||
                                 8 == bit || 10 == bit || 17 == bit))
        {
            return true;
        }

        if (iohs_dlp_fir_oc == id && (54 <= bit && bit <= 61))
        {
            return true;
        }

        if (iohs_dlp_fir_smp == id && (54 <= bit && bit <= 61))
        {
            return true;
        }

        if (nx_cq_fir == id && (7 == bit || 16 == bit || 21 == bit))
        {
            return true;
        }

        if (nx_dma_eng_fir == id && (0 == bit))
        {
            return true;
        }

        if (pau_fir_0 == id &&
            (15 == bit || 18 == bit || 19 == bit || 25 == bit || 26 == bit ||
             29 == bit || 33 == bit || 34 == bit || 35 == bit || 40 == bit ||
             42 == bit || 44 == bit || 45 == bit))
        {
            return true;
        }

        if (pau_fir_1 == id &&
            (13 == bit || 14 == bit || 15 == bit || 37 == bit || 39 == bit ||
             40 == bit || 41 == bit || 42 == bit))
        {
            return true;
        }

        if (pau_fir_2 == id &&
            ((4 <= bit && bit <= 18) || (20 <= bit && bit <= 31) ||
             (36 <= bit && bit <= 41) || 45 == bit || 47 == bit || 48 == bit ||
             50 == bit || 51 == bit || 52 == bit))
        {
            return true;
        }

        if (pau_ptl_fir == id && (4 == bit || 8 == bit))
        {
            return true;
        }
    }
    else if (TYPE_OCMB == targetType)
    {
        if (rdffir == id && (14 == bit || 15 == bit || 17 == bit || 37 == bit))
        {
            return true;
        }
    }

    return false; // default, nothing found
}

//------------------------------------------------------------------------------

bool __findCsRootCause_RE(const std::vector<libhei::Signature>& i_list,
                          libhei::Signature& o_rootCause)
{
    for (const auto s : i_list)
    {
        // Only looking for recoverable attentions.
        if (libhei::ATTN_TYPE_RECOVERABLE != s.getAttnType())
        {
            continue;
        }

        if (__findCsRootCause(s))
        {
            o_rootCause = s;
            return true;
        }
    }

    return false; // default, nothing found
}

//------------------------------------------------------------------------------

bool __findCsRootCause_UCS(const std::vector<libhei::Signature>& i_list,
                           libhei::Signature& o_rootCause)
{
    for (const auto s : i_list)
    {
        // Only looking for unit checkstop attentions.
        if (libhei::ATTN_TYPE_UNIT_CS != s.getAttnType())
        {
            continue;
        }

        if (__findCsRootCause(s))
        {
            o_rootCause = s;
            return true;
        }
    }

    return false; // default, nothing found
}

//------------------------------------------------------------------------------

bool __findNonExternalCs(const std::vector<libhei::Signature>& i_list,
                         libhei::Signature& o_rootCause)
{
    using namespace util::pdbg;

    static const auto pb_ext_fir = libhei::hash<libhei::NodeId_t>("PB_EXT_FIR");

    for (const auto s : i_list)
    {
        const auto targetType = getTrgtType(getTrgt(s.getChip()));
        const auto id         = s.getId();
        const auto attnType   = s.getAttnType();

        // Find any processor with system checkstop attention that did not
        // originate from the PB_EXT_FIR.
        if ((TYPE_PROC == targetType) &&
            (libhei::ATTN_TYPE_CHECKSTOP == attnType) && (pb_ext_fir != id))
        {
            o_rootCause = s;
            return true;
        }
    }

    return false; // default, nothing found
}

//------------------------------------------------------------------------------

bool filterRootCause(AnalysisType i_type,
                     const libhei::IsolationData& i_isoData,
                     libhei::Signature& o_rootCause)
{
    // We'll need to make a copy of the list so that the original list is
    // maintained for the PEL.
    std::vector<libhei::Signature> list{i_isoData.getSignatureList()};

    // START WORKAROUND
    // TODO: Filtering should be data driven. Until that support is available,
    //       use the following isolation rules.

    // Ensure the list is not empty before continuing.
    if (list.empty())
    {
        return false; // nothing more to do
    }

    // First, look for any RCS OSC errors. This must always be first because
    // they can cause downstream PLL unlock attentions.
    if (__findRcsOscError(list, o_rootCause))
    {
        return true;
    }

    // Second, look for any PLL unlock attentions. This must always be second
    // because PLL unlock attentions can cause any number of downstream
    // attentions, including a system checkstop.
    if (__findPllUnlock(list, o_rootCause))
    {
        return true;
    }

    // Regardless of the analysis type, always look for anything that could be
    // blamed as the root cause of a system checkstop.

    // Memory channel failure attentions will produce SUEs and likely cause
    // downstream attentions, including a system checkstop.
    if (__findMemoryChannelFailure(list, o_rootCause))
    {
        return true;
    }

    // Look for any recoverable attentions that have been identified as a
    // potential root cause of a system checkstop attention. These would include
    // any attention that would generate an SUE. Note that is it possible for
    // recoverables to generate unit checkstop attentions so we must check them
    // first.
    if (__findCsRootCause_RE(list, o_rootCause))
    {
        return true;
    }

    // Look for any unit checkstop attentions (other than memory channel
    // failures) that have been identified as a potential root cause of a
    // system checkstop attention. These would include any attention that would
    // generate an SUE.
    if (__findCsRootCause_UCS(list, o_rootCause))
    {
        return true;
    }

    // Look for any system checkstop attentions that originated from within the
    // chip that reported the attention. In other words, no external checkstop
    // attentions.
    if (__findNonExternalCs(list, o_rootCause))
    {
        return true;
    }

    if (AnalysisType::SYSTEM_CHECKSTOP != i_type)
    {
        // No system checkstop root cause attentions were found. Next, look for
        // any recoverable or unit checkstop attentions that could be associated
        // with a TI. Also, ignore any attentions from EQ_CORE_FIR[56:57]
        // because they simply indicate an attention came from the other core in
        // the fused core pair.

        auto itr = std::find_if(list.begin(), list.end(), [&](const auto& t) {
            return (
                (libhei::ATTN_TYPE_RECOVERABLE == t.getAttnType() ||
                 libhei::ATTN_TYPE_UNIT_CS == t.getAttnType()) &&
                !(libhei::hash<libhei::NodeId_t>("EQ_CORE_FIR") == t.getId() &&
                  (56 == t.getBit() || 57 == t.getBit())));
        });

        if (list.end() != itr)
        {
            o_rootCause = *itr;
            return true;
        }

        if (AnalysisType::TERMINATE_IMMEDIATE != i_type)
        {
            // No attentions associated with a system checkstop or TI were
            // found. Simply, return the first entry in the list.
            o_rootCause = list.front();
            return true;
        }
    }

    // END WORKAROUND

    return false; // default, no active attentions found.
}

//------------------------------------------------------------------------------

} // namespace analyzer
