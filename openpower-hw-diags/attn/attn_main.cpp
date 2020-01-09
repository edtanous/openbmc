#include <libpdbg.h>

#include <attn_handler.hpp>
#include <attn_monitor.hpp>

/**
 * @brief Attention handler application main()
 *
 * This is the main interface to the Attention handler application, it will
 * initialize the libgpd targets and start a gpio mointor.
 *
 * @return 0 = success
 */
int main()
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
        // Creating a vector of one gpio to monitor
        std::vector<std::unique_ptr<attn::AttnMonitor>> gpios;
        gpios.push_back(std::make_unique<attn::AttnMonitor>(line, config, io));

        io.run(); // start GPIO monitor
    }

    return rc;
}
