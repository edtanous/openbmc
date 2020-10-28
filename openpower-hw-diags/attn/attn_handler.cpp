#include <libpdbg.h>

#include <analyzer/analyzer_main.hpp>
#include <attn/attention.hpp>
#include <attn/attn_config.hpp>
#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <attn/bp_handler.hpp>
#include <attn/ti_handler.hpp>

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

namespace attn
{

/**
 * @brief Handle SBE vital attention
 *
 * @param i_attention Attention object
 * @return 0 indicates that the vital attention was successfully handled
 *         1 indicates that the vital attention was NOT successfully handled
 */
int handleVital(Attention* i_attention);

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
    uint32_t proc;

    // loop through processors looking for active attentions
    trace<level::INFO>("Attention handler started");

    pdbg_target* target;
    pdbg_for_each_class_target("proc", target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(target))
        {
            proc = pdbg_target_index(target); // get processor number

            std::stringstream ss; // log message stream
            ss << "checking processor " << proc;
            trace<level::INFO>(ss.str().c_str());

            // The processor FSI target is required for CFAM read
            char path[16];
            sprintf(path, "/proc%d/fsi", proc);
            pdbg_target* attnTarget = pdbg_target_from_path(nullptr, path);

            if (PDBG_TARGET_ENABLED == pdbg_target_probe(attnTarget))
            {
                // get active attentions on processor
                if (RC_SUCCESS != fsi_read(attnTarget, 0x1007, &isr_val))
                {
                    // event
                    eventAttentionFail(RC_CFAM_ERROR);

                    // trace
                    trace<level::INFO>("Error! cfam read 0x1007 FAILED");
                }
                else
                {
                    std::stringstream ss; // log message stream
                    ss << "cfam 0x1007 = 0x";
                    ss << std::hex << std::setw(8) << std::setfill('0');
                    ss << isr_val;
                    trace<level::INFO>(ss.str().c_str());

                    // get interrupt enabled special attentions mask
                    if (RC_SUCCESS != fsi_read(attnTarget, 0x100d, &isr_mask))
                    {
                        // event
                        eventAttentionFail(RC_CFAM_ERROR);

                        // trace
                        trace<level::INFO>("Error! cfam read 0x100d FAILED");
                    }
                    else
                    {
                        // CFAM 0x100d is expected to have bits set
                        // corresponding to attentions that can generate an
                        // attention interrupt.

                        std::stringstream ss; // log message stream
                        ss << "cfam 0x100d = 0x";
                        ss << std::hex << std::setw(8) << std::setfill('0');
                        ss << isr_mask;
                        trace<level::INFO>(ss.str().c_str());

                        // bit 0 on "left": bit 30 = SBE vital attention
                        if (isr_val & isr_mask & 0x00000002)
                        {
                            active_attentions.emplace_back(Attention::Vital,
                                                           handleVital, target,
                                                           i_config);
                        }

                        // bit 0 on "left": bit 1 = checkstop
                        if (isr_val & isr_mask & 0x40000000)
                        {
                            active_attentions.emplace_back(Attention::Checkstop,
                                                           handleCheckstop,
                                                           target, i_config);
                        }

                        // bit 0 on "left": bit 2 = special attention
                        if (isr_val & isr_mask & 0x20000000)
                        {
                            active_attentions.emplace_back(Attention::Special,
                                                           handleSpecial,
                                                           target, i_config);
                        }
                    } // cfam 0x100d valid
                }     // cfam 0x1007 valid
            }         // proc target enabled
        }             // fsi target enabled
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
 * @brief Handle SBE vital attention
 *
 * @param i_attention Attention object
 * @return 0 indicates that the vital attention was successfully handled
 *         1 indicates that the vital attention was NOT successfully handled
 */
int handleVital(Attention* i_attention)
{
    int rc = RC_SUCCESS; // assume vital handled

    trace<level::INFO>("vital handler started");

    // if vital handling enabled, handle vital attention
    if (false == (i_attention->getConfig()->getFlag(enVital)))
    {
        trace<level::INFO>("vital handling disabled");
        rc = RC_NOT_HANDLED;
    }
    else
    {
        eventVital();
    }

    return rc;
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

    // if checkstop handling enabled, handle checkstop attention
    if (false == (i_attention->getConfig()->getFlag(enCheckstop)))
    {
        trace<level::INFO>("Checkstop handling disabled");
    }
    else
    {
        // Look for any attentions found in hardware. This will generate and
        // comment a PEL if any errors are found.
        if (true != analyzer::analyzeHardware())
        {
            rc = RC_ANALYZER_ERROR;
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

    // The TI infor chipop will give us a pointer to the TI info data
    uint8_t* tiInfo       = nullptr;                  // ptr to TI info data
    uint32_t tiInfoLen    = 0;                        // length of TI info data
    pdbg_target* attnProc = i_attention->getTarget(); // proc with attention

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
                trace<level::INFO>("calling sbe_mpipl_get_ti_info");
                sbe_mpipl_get_ti_info(tiInfoTarget, &tiInfo, &tiInfoLen);
                if (tiInfo == nullptr)
                {
                    trace<level::INFO>("TI info data ptr is null after call");
                }
            }
        }
    }

    // If TI area exists and is marked valid we can assume TI occurred
    if ((nullptr != tiInfo) && (0 != tiInfo[0]))
    {
        trace<level::INFO>("TI info data present and valid");

        TiDataArea* tiDataArea = (TiDataArea*)tiInfo;

        // trace a few known TI data area values
        std::stringstream ss;
        ss << std::hex << std::showbase;

        ss << "TI data command = " << (int)tiDataArea->command;
        trace<level::INFO>(ss.str().c_str());
        ss.str(std::string());

        ss << "TI data hb_terminate_type = "
           << (int)tiDataArea->hbTerminateType;
        trace<level::INFO>(ss.str().c_str());
        ss.str(std::string());

        ss << "TI data SRC format = " << (int)tiDataArea->srcFormat;
        trace<level::INFO>(ss.str().c_str());
        ss.str(std::string());

        ss << "TI data source = " << (int)tiDataArea->source;
        trace<level::INFO>(ss.str().c_str());
        ss.str(std::string());

        if (true == (i_attention->getConfig()->getFlag(enTerminate)))
        {
            trace<level::INFO>("TI (terminate immediately)");

            // Call TI special attention handler
            rc = tiHandler(tiDataArea);
        }
    }
    // TI area not valid or not available
    else
    {
        trace<level::INFO>("TI info NOT available");

        // if configured to handle breakpoint as default special attention
        if (i_attention->getConfig()->getFlag(dfltBreakpoint))
        {
            trace<level::INFO>("assuming breakpoint");

            if (true == (i_attention->getConfig()->getFlag(enBreakpoints)))
            {
                trace<level::INFO>("breakpoint");

                // Call the breakpoint special attention handler
                bpHandler();
            }
        }
        // if configured to handle TI as default special attention
        else
        {
            trace<level::INFO>("assuming TI");

            if (true == (i_attention->getConfig()->getFlag(enTerminate)))
            {
                trace<level::INFO>("TI (terminate immediately)");

                // Call TI special attention handler
                rc = tiHandler(nullptr);
            }
        }
    }

    // release TI data buffer
    if (nullptr != tiInfo)
    {
        free(tiInfo);
    }

    if (RC_SUCCESS != rc)
    {
        trace<level::INFO>("Special attn not handled");
    }

    return rc;
}

} // namespace attn
