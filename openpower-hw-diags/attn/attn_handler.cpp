extern "C"
{
#include <libpdbg.h>
#include <libpdbg_sbe.h>
}

#include <analyzer/analyzer_main.hpp>
#include <attn/attention.hpp>
#include <attn/attn_common.hpp>
#include <attn/attn_config.hpp>
#include <attn/attn_dbus.hpp>
#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <attn/bp_handler.hpp>
#include <attn/ti_handler.hpp>
#include <attn/vital_handler.hpp>
#include <util/dbus.hpp>

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

namespace attn
{

/**
 * @brief Handle checkstop attention
 *
 * @param i_attention Attention object
 * @return 0 indicates that the checkstop attention was successfully handled
 *         1 indicates that the checkstop attention was NOT successfully
 *           handled.
 */
int handleCheckstop(Attention* i_attention);

/**
 * @brief Handle special attention
 *
 * @param i_attention Attention object
 * @return 0 indicates that the special attention was successfully handled
 *         1 indicates that the special attention was NOT successfully handled
 */
int handleSpecial(Attention* i_attention);

/** @brief Determine if attention is active and not masked */
bool activeAttn(uint32_t i_val, uint32_t i_mask, uint32_t i_attn);

/**
 * @brief The main attention handler logic
 *
 * @param i_breakpoints true = breakpoint special attn handling enabled
 */
void attnHandler(Config* i_config)
{
    // Vector of active attentions to be handled
    std::vector<Attention> active_attentions;

    uint32_t isr_val, isr_mask;

    // loop through processors looking for active attentions
    trace<level::INFO>("Attention handler started");

    pdbg_target* target;
    pdbg_for_each_class_target("proc", target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(target))
        {
            auto proc = pdbg_target_index(target); // get processor number

            // Use PIB target to determine if a processor is enabled
            char path[16];
            sprintf(path, "/proc%d/pib", proc);
            pdbg_target* pibTarget = pdbg_target_from_path(nullptr, path);

            // sanity check
            if (nullptr == pibTarget)
            {
                trace<level::INFO>("pib path or target not found");
                continue;
            }

            // check if pib target is enabled
            if (PDBG_TARGET_ENABLED == pdbg_target_probe(pibTarget))
            {
                // The processor FSI target is required for CFAM read
                sprintf(path, "/proc%d/fsi", proc);
                pdbg_target* fsiTarget = pdbg_target_from_path(nullptr, path);

                // sanity check
                if (nullptr == fsiTarget)
                {
                    trace<level::INFO>("fsi path or target not found");
                    continue;
                }

                // trace the proc number
                std::string traceMsg = "proc: " + std::to_string(proc);
                trace<level::INFO>(traceMsg.c_str());

                isr_val = 0xffffffff; // invalid isr value

                // get active attentions on processor
                if (RC_SUCCESS != fsi_read(fsiTarget, 0x1007, &isr_val))
                {
                    // log cfam read error
                    trace<level::INFO>("Error! cfam read 0x1007 FAILED");
                    eventAttentionFail((int)AttnSection::attnHandler |
                                       ATTN_PDBG_CFAM);
                }
                else if (0xffffffff == isr_val)
                {
                    trace<level::INFO>("Error! cfam read 0x1007 INVALID");
                    continue;
                }
                else
                {
                    // trace isr value
                    std::stringstream ssIsr;
                    ssIsr << "cfam 0x1007 = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << isr_val;
                    std::string strobjIsr = ssIsr.str();
                    trace<level::INFO>(strobjIsr.c_str());

                    isr_mask = 0xffffffff; // invalid isr mask

                    // get interrupt enabled special attentions mask
                    if (RC_SUCCESS != fsi_read(fsiTarget, 0x100d, &isr_mask))
                    {
                        // log cfam read error
                        trace<level::INFO>("Error! cfam read 0x100d FAILED");
                        eventAttentionFail((int)AttnSection::attnHandler |
                                           ATTN_PDBG_CFAM);
                    }
                    else if (0xffffffff == isr_mask)
                    {
                        trace<level::INFO>("Error! cfam read 0x100d INVALID");
                        continue;
                    }
                    else
                    {
                        // trace true mask
                        std::stringstream ssMask;
                        ssMask << "cfam 0x100d = 0x" << std::setw(8)
                               << std::setfill('0') << std::hex << isr_mask;
                        std::string strobjMask = ssMask.str();
                        trace<level::INFO>(strobjMask.c_str());

                        // SBE vital attention active and not masked?
                        if (true == activeAttn(isr_val, isr_mask, SBE_ATTN))
                        {
                            active_attentions.emplace_back(Attention::Vital,
                                                           handleVital, target,
                                                           i_config);
                        }

                        // Checkstop attention active and not masked?
                        if (true ==
                            activeAttn(isr_val, isr_mask, CHECKSTOP_ATTN))
                        {
                            active_attentions.emplace_back(Attention::Checkstop,
                                                           handleCheckstop,
                                                           target, i_config);
                        }

                        // Special attention active and not masked?
                        if (true == activeAttn(isr_val, isr_mask, SPECIAL_ATTN))
                        {
                            active_attentions.emplace_back(Attention::Special,
                                                           handleSpecial,
                                                           target, i_config);
                        }
                    } // cfam 0x100d valid
                }     // cfam 0x1007 valid
            }         // fsi target enabled
        }             // pib target enabled
    }                 // next processor

    // convert to heap, highest priority is at front
    if (!std::is_heap(active_attentions.begin(), active_attentions.end()))
    {
        std::make_heap(active_attentions.begin(), active_attentions.end());
    }

    // call the attention handler until one is handled or all were attempted
    while (false == active_attentions.empty())
    {
        // handle highest priority attention, done if successful
        if (RC_SUCCESS == active_attentions.front().handle())
        {
            // an attention was handled so we are done
            break;
        }

        // move attention to back of vector
        std::pop_heap(active_attentions.begin(), active_attentions.end());

        // remove attention from vector
        active_attentions.pop_back();
    }
}

/**
 * @brief Handle checkstop attention
 *
 * @param i_attention Attention object
 * @return 0 indicates that the checkstop attention was successfully handled
 *         1 indicates that the checkstop attention was NOT successfully
 *           handled.
 */
int handleCheckstop(Attention* i_attention)
{
    int rc = RC_SUCCESS; // assume checkstop handled

    trace<level::INFO>("checkstop handler started");

    // capture some additional data for logs/traces
    addHbStatusRegs();

    // if checkstop handling enabled, handle checkstop attention
    if (false == (i_attention->getConfig()->getFlag(enCheckstop)))
    {
        trace<level::INFO>("Checkstop handling disabled");
    }
    else
    {
        // Look for any attentions found in hardware. This will generate and
        // commit a PEL if any errors are found.
        DumpParameters dumpParameters;
        if (true != analyzer::analyzeHardware(dumpParameters))
        {
            rc = RC_ANALYZER_ERROR;
        }
        else
        {
            requestDump(dumpParameters);
            util::dbus::transitionHost(util::dbus::HostState::Quiesce);
        }
    }

    return rc;
}

/**
 * @brief Handle special attention
 *
 * @param i_attention Attention object
 * @return 0 indicates that the special attention was successfully handled
 *         1 indicates that the special attention was NOT successfully handled
 */
int handleSpecial(Attention* i_attention)
{
    int rc = RC_SUCCESS; // assume special attention handled

    // The TI info chipop will give us a pointer to the TI info data
    uint8_t* tiInfo       = nullptr;                  // ptr to TI info data
    uint32_t tiInfoLen    = 0;                        // length of TI info data
    pdbg_target* attnProc = i_attention->getTarget(); // proc with attention

    bool tiInfoStatic = false; // assume TI info was provided (not created)

    if (attnProc != nullptr)
    {
        // The processor PIB target is required for get TI info chipop
        char path[16];
        sprintf(path, "/proc%d/pib", pdbg_target_index(attnProc));
        pdbg_target* tiInfoTarget = pdbg_target_from_path(nullptr, path);

        if (nullptr != tiInfoTarget)
        {
            if (PDBG_TARGET_ENABLED == pdbg_target_probe(tiInfoTarget))
            {
                sbe_mpipl_get_ti_info(tiInfoTarget, &tiInfo, &tiInfoLen);

                // If TI info not available use default based on host state
                if (nullptr == tiInfo)
                {
                    trace<level::INFO>("TI info data ptr is invalid");

                    util::dbus::HostRunningState runningState =
                        util::dbus::hostRunningState();

                    std::string stateString = "host state unknown";

                    if ((util::dbus::HostRunningState::Started ==
                         runningState) ||
                        (util::dbus::HostRunningState::Unknown == runningState))
                    {
                        if (util::dbus::HostRunningState::Started ==
                            runningState)
                        {
                            stateString = "host started";
                        }
                        tiInfo = (uint8_t*)defaultPhypTiInfo;
                    }
                    else
                    {
                        stateString = "host not started";
                        tiInfo      = (uint8_t*)defaultHbTiInfo;
                    }

                    // trace host state
                    trace<level::INFO>(stateString.c_str());

                    tiInfoStatic = true; // using our TI info
                }
            }
        }
    }

    bool tiInfoValid = false; // TI area data not valid or not available

    // If TI area exists and is marked valid we can assume TI occurred
    if ((nullptr != tiInfo) && (0 != tiInfo[0]))
    {
        TiDataArea* tiDataArea = (TiDataArea*)tiInfo;

        std::stringstream ss; // string stream object for tracing
        std::string strobj;   // string object for tracing

        // trace a few known TI data area values
        ss.str(std::string()); // empty the stream
        ss.clear();            // clear the stream
        ss << "TI data command = 0x" << std::setw(2) << std::setfill('0')
           << std::hex << (int)tiDataArea->command;
        strobj = ss.str();
        trace<level::INFO>(strobj.c_str());

        // Another check for valid TI Info since it has been seen that
        // tiInfo[0] != 0 but TI info is not valid
        if (0xa1 == tiDataArea->command)
        {
            tiInfoValid = true;

            // trace some more data since TI info appears valid
            ss.str(std::string()); // empty the stream
            ss.clear();            // clear the stream
            ss << "TI data term-type = 0x" << std::setw(2) << std::setfill('0')
               << std::hex << (int)tiDataArea->hbTerminateType;
            strobj = ss.str();
            trace<level::INFO>(strobj.c_str());

            ss.str(std::string()); // empty the stream
            ss.clear();            // clear the stream
            ss << "TI data SRC format = 0x" << std::setw(2) << std::setfill('0')
               << std::hex << (int)tiDataArea->srcFormat;
            strobj = ss.str();
            trace<level::INFO>(strobj.c_str());

            ss.str(std::string()); // empty the stream
            ss.clear();            // clear the stream
            ss << "TI data source = 0x" << std::setw(2) << std::setfill('0')
               << std::hex << (int)tiDataArea->source;
            strobj = ss.str();
            trace<level::INFO>(strobj.c_str());

            if (true == (i_attention->getConfig()->getFlag(enTerminate)))
            {
                // Call TI special attention handler
                rc = tiHandler(tiDataArea);
            }
        }
    }

    // TI area is available but was not valid data
    if (false == tiInfoValid)
    {
        trace<level::INFO>("TI info NOT valid");

        // if configured to handle TI as default special attention
        if (i_attention->getConfig()->getFlag(dfltTi))
        {
            // TI handling may be disabled
            if (true == (i_attention->getConfig()->getFlag(enTerminate)))
            {
                // Call TI special attention handler
                rc = tiHandler(nullptr);
            }
        }
        // breakpoint is default special attention when TI info NOT valid
        else
        {
            // breakpoint handling may be disabled
            if (true == (i_attention->getConfig()->getFlag(enBreakpoints)))
            {
                // Call the breakpoint special attention handler
                rc = bpHandler();
            }
        }
    }

    // release TI data buffer
    if ((nullptr != tiInfo) && (false == tiInfoStatic))
    {
        free(tiInfo);
    }

    if (RC_SUCCESS != rc)
    {
        trace<level::INFO>("Special attn not handled");
    }

    return rc;
}

/**
 * @brief Determine if attention is active and not masked
 *
 * Determine whether an attention needs to be handled and trace details of
 * attention type and whether it is masked or not.
 *
 * @param i_val attention status register
 * @param i_mask attention true mask register
 * @param i_attn attention type
 * @param i_proc processor associated with registers
 *
 * @return true if attention is active and not masked, otherwise false
 */
bool activeAttn(uint32_t i_val, uint32_t i_mask, uint32_t i_attn)
{
    bool rc = false; // assume attn masked and/or inactive

    // if attention active
    if (0 != (i_val & i_attn))
    {
        std::stringstream ss;

        bool validAttn = true; // known attention type

        switch (i_attn)
        {
            case SBE_ATTN:
                ss << "SBE attn";
                break;
            case CHECKSTOP_ATTN:
                ss << "Checkstop attn";
                break;
            case SPECIAL_ATTN:
                ss << "Special attn";
                break;
            default:
                ss << "Unknown attn";
                validAttn = false;
        }

        // see if attention is masked
        if (true == validAttn)
        {
            if (0 != (i_mask & i_attn))
            {
                rc = true; // attention active and not masked
            }
            else
            {
                ss << " masked";
            }
        }

        std::string strobj = ss.str();      // ss.str() is temporary
        trace<level::INFO>(strobj.c_str()); // commit trace stream
    }

    return rc;
}
} // namespace attn
