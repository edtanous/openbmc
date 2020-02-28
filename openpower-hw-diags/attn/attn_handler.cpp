#include <libpdbg.h>

#include <analyzer/analyzer_main.hpp>
#include <bp_handler.hpp>
#include <logging.hpp>
#include <ti_handler.hpp>

#include <iomanip>
#include <sstream>

namespace attn
{

/**
 * @brief Handle SBE vital attention
 *
 * @return 0 = success
 */
int handleVital();

/**
 * @brief Handle checkstop attention
 *
 * @return 0 = success
 */
int handleCheckstop();

/**
 * @brief Handle special attention
 *
 * @param i_breakpoints true = breakpoint special attn handling enabled
 * @return 0 = success
 */
int handleSpecial(bool i_breakpoints);

/**
 * @brief The main attention handler logic
 *
 * @param i_breakpoints true = breakpoint special attn handling enabled
 */
void attnHandler(bool i_breakpoints)
{
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
            ss << "checking processor " << proc << std::endl;
            log<level::INFO>(ss.str().c_str());

            // get active attentions on processor
            if (0 != fsi_read(target, 0x1007, &isr_val))
            {
                std::stringstream ss; // log message stream
                ss << "Error! cfam read 0x1007 FAILED" << std::endl;
                log<level::INFO>(ss.str().c_str());
            }
            else
            {
                std::stringstream ss; // log message stream
                ss << "cfam 0x1007 = 0x";
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << isr_val << std::endl;
                log<level::INFO>(ss.str().c_str());

                // get interrupt enabled special attentions mask
                if (0 != fsi_read(target, 0x100d, &isr_mask))
                {
                    std::stringstream ss; // log message stream
                    ss << "Error! cfam read 0x100d FAILED" << std::endl;
                    log<level::INFO>(ss.str().c_str());
                }
                else
                {
                    std::stringstream ss; // log message stream
                    ss << "cfam 0x100d = 0x";
                    ss << std::hex << std::setw(8) << std::setfill('0');
                    ss << isr_mask << std::endl;
                    log<level::INFO>(ss.str().c_str());

                    // bit 0 on "left": bit 30 = SBE vital attention
                    if (isr_val & isr_mask & 0x00000002)
                    {
                        handleVital();
                    }

                    // bit 0 on "left": bit 1 = checkstop
                    if (isr_val & isr_mask & 0x40000000)
                    {
                        if (0 == handleCheckstop())
                        {
                            break;
                        }
                    }

                    // bit 0 on "left": bit 2 = special attention
                    if (isr_val & isr_mask & 0x20000000)
                    {
                        handleSpecial(i_breakpoints);
                    }
                } // cfam 0x100d valid
            }     // cfam 0x1007 valid
        }         // fsi target enabled
    }             // next processor

    return; // checked all processors
}

/**
 * @brief Handle SBE vital attention
 */
int handleVital()
{
    int rc = 1; // vital attention handling not yet supported

    std::stringstream ss; // log message stream
    ss << "vital" << std::endl;
    log<level::INFO>(ss.str().c_str());

    if (0 != rc)
    {
        std::stringstream ss; // log message stream
        ss << "vital NOT handled" << std::endl;
        log<level::INFO>(ss.str().c_str());
    }

    return rc;
}

/**
 * @brief Handle checkstop attention
 */
int handleCheckstop()
{
    int rc = 0; // checkstop handling supported

    std::stringstream ss; // log message stream
    ss << "checkstop" << std::endl;
    log<level::INFO>(ss.str().c_str());

    analyzer::analyzeHardware();

    // TODO recoverable errors?

    return rc;
}

/**
 * @brief Handle special attention
 *
 * @param i_breakpoints true = breakpoint special attn handling enabled
 */
int handleSpecial(bool i_breakpoints)
{
    int rc = 0; // special attention handling supported

    std::stringstream ss; // log message stream

    ss << "special" << std::endl;

    // Right now we always handle breakpoint special attentions if breakpoint
    // attn handling is enabled. This will eventually check if breakpoint attn
    // handing is enabled AND there is a breakpoint pending.
    if (true == i_breakpoints)
    {
        ss << "breakpoint" << std::endl;
        log<level::INFO>(ss.str().c_str());

        // Call the breakpoint special attention handler
        bpHandler();
    }
    // Right now if breakpoint attn handling is not enabled we will treat the
    // special attention as a TI. This will eventually be changed to check
    // whether a TI is active and handle it regardless of whether breakpoint
    // handling is enbaled or not.
    else
    {
        ss << "TI (terminate immediately)" << std::endl;
        log<level::INFO>(ss.str().c_str());

        // Call TI special attention handler
        tiHandler();
    }

    // TODO recoverable errors?

    return rc;
}

} // namespace attn
