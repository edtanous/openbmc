#include <libpdbg.h>

#include <analyzer/analyzer_main.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <cli.hpp>
#include <listener.hpp>

/**
 * @brief Attention handler application main()
 *
 * This is the main interface to the hardware diagnostics application. This
 * application will either be loaded as a daemon for monitoring the attention
 * gpio or it will be loaded as an application to analyze hardware and
 * diagnose hadrware error conditions.
 *
 *     Usage:
 *        --analyze:              Analyze the hardware
 *        --start:                Start the attention handler
 *        --stop:                 Stop the attention handler
 *        --all <on|off>:         All attention handling
 *        --vital <on|off>:       Vital attention handling
 *        --checkstop <on|off>:   Checkstop attention handling
 *        --terminate <on|off>:   Terminate Immiediately attention handling
 *        --breakpoints <on|off>: Breakpoint attention handling
 *
 *     Example: openpower-hw-diags --start --vital off
 *
 * @return 0 = success
 */
int main(int argc, char* argv[])
{
    int rc = RC_SUCCESS; // assume success

    using namespace boost::interprocess;

    if (argc == 1)
    {
        printf("openpower-hw-diags <options>\n");
        printf("options:\n");
        printf("  --analyze:              Analyze the hardware\n");
        printf("  --start:                Start the attention handler\n");
        printf("  --stop:                 Stop the attention handler\n");
        printf("  --all <on|off>:         All attention handling\n");
        printf("  --vital <on|off>:       Vital attention handling\n");
        printf("  --checkstop <on|off>:   Checkstop attention handling\n");
        printf("  --terminate <on|off>:   Terminate Immediately attention "
               "handling\n");
        printf("  --breakpoints <on|off>: Breakpoint attention handling\n");
    }
    else
    {
        // Pdbg targets should only be initialized once according to
        // libpdbg documentation. Initializing them here will make sure
        // they are initialized for the attention handler, invocation of
        // the analyzer via attention handler and direct invocation of
        // the analyzer via command line (--analyze).

        pdbg_targets_init(nullptr); // nullptr == use default fdt

        // Either analyze (application mode) or daemon mode
        if (true == getCliOption(argv, argv + argc, "--analyze"))
        {
            rc = analyzer::analyzeHardware(); // analyze hardware
        }
        // daemon mode
        else
        {
            // Handle pending attentions
            attn::Config attnConfig;
            attn::attnHandler(&attnConfig);

            // assume listener is not running
            bool listenerStarted = false;
            bool newListener     = false;

            pthread_t ptidListener; // handle to listener thread

            // see if listener is already started
            listenerStarted = listenerMqExists();

            // listener is not running so start it
            if (false == listenerStarted)
            {
                // create listener thread
                if (0 ==
                    pthread_create(&ptidListener, NULL, &threadListener, NULL))
                {
                    listenerStarted = true;
                    newListener     = true;
                }
                else
                {
                    rc = 1;
                }
            }

            // listener was running or just started
            if (true == listenerStarted)
            {
                // If we created a new listener this instance of
                // openpower-hw-diags will become our daemon (it will not exit
                // until stopped).
                if (true == newListener)
                {
                    bool listenerReady = false;

                    // It may take some time for the listener to become ready,
                    // we will wait until the message queue has been created
                    // before starting to communicate with our daemon.
                    while (false == listenerReady)
                    {
                        usleep(500);
                        listenerReady = listenerMqExists();
                    }
                }

                // send cmd line to listener thread
                if (argc != sendCmdLine(argc, argv))
                {
                    rc = 1;
                }

                // if this is a new listener let it run until "stopped"
                if (true == newListener)
                {
                    pthread_join(ptidListener, NULL);
                }
            }
        }
    }
    return rc;
}
