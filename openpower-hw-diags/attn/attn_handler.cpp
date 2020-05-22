#include <analyzer/analyzer_main.hpp>
#include <attention.hpp>
#include <attn_config.hpp>
#include <bp_handler.hpp>
#include <logging.hpp>
#include <ti_handler.hpp>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>

namespace attn
{

/** @brief Return codes */
static constexpr int RC_SUCCESS     = 0;
static constexpr int RC_NOT_SUCCESS = 1;

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
    log<level::INFO>("Attention handler started");

    pdbg_target* target;
    pdbg_for_each_class_target("fsi", target)
    {
        log<level::INFO>("iterating targets");
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(target))
        {
            proc = pdbg_target_index(target); // get processor number

            std::stringstream ss; // log message stream
            ss << "checking processor " << proc;
            log<level::INFO>(ss.str().c_str());

            // get active attentions on processor
            if (RC_SUCCESS != fsi_read(target, 0x1007, &isr_val))
            {
                log<level::INFO>("Error! cfam read 0x1007 FAILED");
            }
            else
            {
                std::stringstream ss; // log message stream
                ss << "cfam 0x1007 = 0x";
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << isr_val;
                log<level::INFO>(ss.str().c_str());

                // get interrupt enabled special attentions mask
                if (RC_SUCCESS != fsi_read(target, 0x100d, &isr_mask))
                {
                    log<level::INFO>("Error! cfam read 0x100d FAILED");
                }
                else
                {
                    std::stringstream ss; // log message stream
                    ss << "cfam 0x100d = 0x";
                    ss << std::hex << std::setw(8) << std::setfill('0');
                    ss << isr_mask;
                    log<level::INFO>(ss.str().c_str());

                    // bit 0 on "left": bit 30 = SBE vital attention
                    if (isr_val & isr_mask & 0x00000002)
                    {
                        active_attentions.emplace_back(
                            Attention::Vital, handleVital, target, i_config);
                    }

                    // bit 0 on "left": bit 1 = checkstop
                    if (isr_val & isr_mask & 0x40000000)
                    {
                        active_attentions.emplace_back(Attention::Checkstop,
                                                       handleCheckstop, target,
                                                       i_config);
                    }

                    // bit 0 on "left": bit 2 = special attention
                    if (isr_val & isr_mask & 0x20000000)
                    {
                        active_attentions.emplace_back(Attention::Special,
                                                       handleSpecial, target,
                                                       i_config);
                    }
                } // cfam 0x100d valid
            }     // cfam 0x1007 valid
        }         // fsi target enabled
    }             // next processor

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

    // if vital handling enabled, handle vital attention
    if (false == (i_attention->getConfig()->getFlag(enVital)))
    {
        log<level::INFO>("vital handling disabled");
        rc = RC_NOT_SUCCESS;
    }
    else
    {
        log<level::INFO>("vital NOT handled");
        rc = RC_NOT_SUCCESS;
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

    // if checkstop handling enabled, handle checkstop attention
    if (false == (i_attention->getConfig()->getFlag(enCheckstop)))
    {
        log<level::INFO>("Checkstop handling disabled");
        rc = RC_NOT_SUCCESS;
    }
    else
    {
        analyzer::analyzeHardware();
        rc = RC_SUCCESS;
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
    int rc = RC_NOT_SUCCESS; // assume special attention handling disabled

    // Until the special attention chipop is availabe we will treat the special
    // attention as a TI. If TI handling is disabled we will treat the special
    // attention as a breakpopint.

    // TI attention gets priority over breakpoints, if enabled then handle
    if (true == (i_attention->getConfig()->getFlag(enTerminate)))
    {
        log<level::INFO>("TI (terminate immediately)");

        // Call TI special attention handler
        tiHandler();
        rc = RC_SUCCESS;
    }
    else
    {
        if (true == (i_attention->getConfig()->getFlag(enBreakpoints)))
        {
            log<level::INFO>("breakpoint");

            // Call the breakpoint special attention handler
            bpHandler();
            rc = RC_SUCCESS;
        }
    }

    if (RC_SUCCESS != rc)
    {
        log<level::INFO>("Special attn handling disabled");
    }

    return rc;
}

} // namespace attn
