#include <attn/attn_monitor.hpp>

namespace attn
{

/**
 * @brief Attention handler application main()
 */
int attnDaemon(Config* i_config)
{
    int rc = 0; // assume success

    gpiod_line* line; // gpio line to monitor

    boost::asio::io_service io; // async io monitoring service

    // GPIO line configuration (falling edge, active low)
    struct gpiod_line_request_config config
    {
        "attention", GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE, 0
    };

    // get handle to attention GPIO line
    line = gpiod_line_find("checkstop");

    if (nullptr == line)
    {
        rc = 1; // error
    }
    else
    {
        // Creating a vector of one gpio to monitor
        std::vector<std::unique_ptr<attn::AttnMonitor>> gpios;
        gpios.push_back(
            std::make_unique<attn::AttnMonitor>(line, config, io, i_config));

        io.run(); // start GPIO monitor

        // done with line, manually close chip (per gpiod api comments)
        gpiod_line_close_chip(line);
    }

    return rc;
}

} // namespace attn
