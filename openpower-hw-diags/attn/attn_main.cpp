#include <libpdbg.h>

#include <attn_handler.hpp>
#include <attn_monitor.hpp>

#include <algorithm>

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
 * This is the main interface to the Attention handler application, it will
 * initialize the libgpd targets and start a gpio mointor.
 *
 * Command line arguments:
 *
 *  breakpoints - enables breakpoint special attn handling
 *
 * @return 0 = success
 */
int main(int argc, char* argv[])
{
    int rc = 0; // return code

    gpiod_line* line; // gpio line to monitor

    boost::asio::io_service io; // async io monitoring service

    // initialize pdbg targets
    pdbg_targets_init(nullptr);

    // GPIO line configuration (falling edge, active low)
    struct gpiod_line_request_config config
    {
        "attention", GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE, 0
    };

    // get handle to attention GPIO line
    line = gpiod_line_get("gpiochip0", 74);

    if (nullptr == line)
    {
        rc = 1; // error
    }
    else
    {
        // Check command line args for breakpoint handling enable option
        bool bp_enable = getCliOption(argv, argv + argc, "-breakpoints");

        // Creating a vector of one gpio to monitor
        std::vector<std::unique_ptr<attn::AttnMonitor>> gpios;
        gpios.push_back(
            std::make_unique<attn::AttnMonitor>(line, config, io, bp_enable));

        io.run(); // start GPIO monitor
    }

    return rc;
}
