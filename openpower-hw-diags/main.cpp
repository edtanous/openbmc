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
 *  analyze         analyze hardware
 *  --daemon        load application as a daemon
 *  --breakpoints   enable breakpoint special attn handling (in daemon mode)
 *
 * @return 0 = success
 */
int main(int argc, char* argv[])
{
    int rc = 0; // return code

    // initialize pdbg targets
    pdbg_targets_init(nullptr);

    // TODO Handle target init fail

    // check if we are being loaded as a daemon
    if (true == getCliOption(argv, argv + argc, "--daemon"))
    {
        // Check command line args for breakpoint handling enable option
        bool bp_enable = getCliOption(argv, argv + argc, "--breakpoints");

        // Configure and start attention monitor
        attn::attnDaemon(bp_enable);
    }
    // We are being loaded as an application, so parse the command line
    // arguments to determine what operation is being requested.
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
