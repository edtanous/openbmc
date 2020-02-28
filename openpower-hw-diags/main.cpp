#include <libpdbg.h>

#include <analyzer/analyzer_main.hpp>
#include <attn/attn_main.hpp>

#include <algorithm>
#include <string>

/*
 * @brief Search the command line arguments for an option
 *
 * @param i_begin   command line args vector begin
 * @param i_end     command line args vector end
 * @param i_option  configuration option to look for
 *
 * @return true = option found on command line
 */
bool getCliOption(char** i_begin, char** i_end, const std::string& i_option)
{
    return (i_end != std::find(i_begin, i_end, i_option));
}

/*
 * @brief Search the command line arguments for a setting value
 *
 * @param i_begin   command line args vector begin
 * @param i_end     command line args vectory end
 * @param i_setting configuration setting to look for
 *
 * @return value of the setting or 0 if setting not found or value not given
 */
char* getCliSetting(char** i_begin, char** i_end, const std::string& i_setting)
{
    char** value = std::find(i_begin, i_end, i_setting);
    if (value != i_end && ++value != i_end)
    {
        return *value;
    }
    return 0; // nullptr
}

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
