#include <analyzer/analyzer_main.hpp>
#include <attention.hpp>
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
 * @return 0 = success
 */
int handleVital(Attention* i_attention);

/**
 * @brief Handle checkstop attention
 *
 * @param i_attention Attention object
 * @return 0 = success
 */
int handleCheckstop(Attention* i_attention);

/**
 * @brief Handle special attention
 *
 * @param i_attention Attention object
 * @return 0 = success
 */
int handleSpecial(Attention* i_attention);

/**
 * @brief The main attention handler logic
 *
 * @param i_breakpoints true = breakpoint special attn handling enabled
 */
void attnHandler(const bool i_breakpoints)
{
    // Vector of active attentions to be handled
    std::vector<Attention> active_attentions;

    uint32_t isr_val, isr_mask;
    uint32_t proc;

    // loop through processors looking for active attentions
    pdbg_target* target;
    pdbg_for_each_class_target("fsi", target)
    {
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
                        active_attentions.emplace_back(Attention::Vital,
                                                       handleVital, target,
                                                       i_breakpoints);
                    }

                    // bit 0 on "left": bit 1 = checkstop
                    if (isr_val & isr_mask & 0x40000000)
                    {
                        active_attentions.emplace_back(Attention::Checkstop,
                                                       handleCheckstop, target,
                                                       i_breakpoints);
                    }

                    // bit 0 on "left": bit 2 = special attention
                    if (isr_val & isr_mask & 0x20000000)
                    {
                        active_attentions.emplace_back(Attention::Special,
                                                       handleSpecial, target,
                                                       i_breakpoints);
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
 */
int handleVital(Attention* i_attention)
{
    int rc = RC_NOT_SUCCESS; // vital attention handling not yet supported

    log<level::INFO>("vital");

    if (RC_SUCCESS != rc)
    {
        log<level::INFO>("vital NOT handled");
    }

    return rc;
}

/**
 * @brief Handle checkstop attention
 */
int handleCheckstop(Attention* i_attention)
{
    int rc = RC_SUCCESS; // checkstop handling supported

    log<level::INFO>("checkstop");

    analyzer::analyzeHardware();

    return rc;
}

/**
 * @brief Handle special attention
 */
int handleSpecial(Attention* i_attention)
{
    int rc = RC_SUCCESS; // special attention handling supported

    log<level::INFO>("special");

    // Right now we always handle breakpoint special attentions if breakpoint
    // attn handling is enabled. This will eventually check if breakpoint attn
    // handing is enabled AND there is a breakpoint pending.
    if (0 != (i_attention->getFlags() & enableBreakpoints))
    {
        log<level::INFO>("breakpoint");

        // Call the breakpoint special attention handler
        bpHandler();
    }
    // Right now if breakpoint attn handling is not enabled we will treat the
    // special attention as a TI. This will eventually be changed to check
    // whether a TI is active and handle it regardless of whether breakpoint
    // handling is enbaled or not.
    else
    {
        log<level::INFO>("TI (terminate immediately)");

        // Call TI special attention handler
        tiHandler();
    }

    // TODO recoverable errors?

    return rc;
}

} // namespace attn
