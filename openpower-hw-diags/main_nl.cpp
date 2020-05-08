#include <libpdbg.h>

#include <analyzer/analyzer_main.hpp>
#include <attn/attention.hpp>
#include <attn/attn_config.hpp>
#include <attn/attn_handler.hpp>
#include <attn/attn_main.hpp>
#include <cli.hpp>

// FIXME TEMP CODE - begin

namespace attn
{
int handleCheckstop(Attention* i_attention);
} // namespace attn

// FIXME TEMP CODE - end

//
/**
 * @brief Attention handler application main()
 *
 * This is the main interface to the hardware diagnostics application. This
 * application can be loaded as a daemon for monitoring the attention
 * gpio or it can be loaded as an application to analyze hardware and
 * diagnose hardware error conditions.
 *
 *     Usage:
 *        --analyze:              Analyze the hardware
 *        --daemon:               Start the attention handler daemon
 *
 * @return 0 = success
 */
int main(int argc, char* argv[])
{
    int rc = 0; // assume success

    if (argc == 1)
    {
        printf("openpower-hw-diags <options>\n");
        printf("options:\n");
        printf("  --analyze:              Analyze the hardware\n");
        printf("  --daemon:               Start the attn handler daemon\n");
    }
    else
    {
        // Either analyze (application mode) or daemon mode
        if (true == getCliOption(argv, argv + argc, "--analyze"))
        {
            // errors that were isolated
            std::map<std::string, std::string> errors;

            rc = analyzer::analyzeHardware(errors); // analyze hardware

            printf("analyzer isolated %i errors", (int)errors.size());
        }
        // daemon mode
        else
        {
            if (true == getCliOption(argv, argv + argc, "--daemon"))
            {
                attn::Config attnConfig; // default config

                pdbg_targets_init(nullptr); // initialize pdbg targets

                attn::attnDaemon(&attnConfig); // start daemon
            }
        }
    }

    return rc;
}
