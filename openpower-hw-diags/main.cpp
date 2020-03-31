#include <libpdbg.h>

#include <analyzer/analyzer_main.hpp>
#include <attn/attn_main.hpp>
#include <cli.hpp>

/**
 * @brief Attention handler application main()
 *
 * This is the main interface to the hardware diagnostics application. This
 * application will either be loaded as a daemon for monitoring the attention
 * gpio or it will be loaded as an application to analyze hardware and
 * diagnose hadrware error conditions.
 *
 * Command line arguments:
 *
 * commands:
 *
 *   analyze            analyze hardware
 *
 * options:
 *
 *  --daemon            load application as a daemon
 *  --vital off         disable vital attention handling (daemon mode)
 *  --checkstop off     disable checkstop attention handling (daemon mode)
 *  --terminate off     disable TI attention handling (daemon mode)
 *  --breakpoints off   disable breakpoint attention handling (daemon mode)
 *
 *  example:
 *
 *    openpower-hw-diags --daemon --terminate off
 *
 * @return 0 = success
 */
int main(int argc, char* argv[])
{
    int rc = 0; // return code

    // attention handler configuration flags
    bool vital_enable     = true;
    bool checkstop_enable = true;
    bool ti_enable        = true;
    bool bp_enable        = true;

    // initialize pdbg targets
    pdbg_targets_init(nullptr);

    // get configuration options
    parseConfig(argv, argv + argc, vital_enable, checkstop_enable, ti_enable,
                bp_enable);

    // check if we are being loaded as a daemon
    if (true == getCliOption(argv, argv + argc, "--daemon"))
    {
        attn::Config config(vital_enable, checkstop_enable, ti_enable,
                            bp_enable);

        // Configure and start attention monitor
        attn::attnDaemon(&config);
    }
    // we are being loaded as an application
    else
    {
        // Request to analyze the hardware for error conditions
        if (true == getCliOption(argv, argv + argc, "analyze"))
        {
            analyzer::analyzeHardware();
        }
    }

    return rc;
}
