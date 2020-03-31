#pragma once

#include <gpiod.h>

#include <attn/attn_config.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

namespace attn
{

/**
 *  @brief Responsible for monitoring attention GPIO state change
 */
class AttnMonitor
{
  public:
    AttnMonitor()  = delete;
    ~AttnMonitor() = default;

    /** @brief Constructs AttnMonitor object.
     *
     * The AttnMonitor constructor will create a new object and start
     * the objects associated GPIO listener.
     *
     * @param line         GPIO line handle
     * @param config       configuration of line
     * @param io           io service
     * @param i_attnConfig poiner to attention handler configuration object
     */
    AttnMonitor(gpiod_line* line, gpiod_line_request_config& config,
                boost::asio::io_service& io, Config* i_attnConfig) :
        iv_gpioLine(line),
        iv_gpioConfig(config), iv_gpioEventDescriptor(io),
        iv_config(i_attnConfig)
    {

        requestGPIOEvent(); // registers the event handler
    }

    // delete copy constructor
    AttnMonitor(const AttnMonitor&) = delete;

    // delete assignment operator
    AttnMonitor& operator=(const AttnMonitor&) = delete;

    // delere move copy consructor
    AttnMonitor(AttnMonitor&&) = delete;

    // delete move assignment operator
    AttnMonitor& operator=(AttnMonitor&&) = delete;

  private: // instance variables
    /** @brief gpiod handle to gpio line */
    gpiod_line* iv_gpioLine;

    /** @brief gpiod line config data */
    gpiod_line_request_config iv_gpioConfig;

    /** @brief GPIO event descriptor */
    boost::asio::posix::stream_descriptor iv_gpioEventDescriptor;

    /** @brief attention handler configuration object pointer */
    Config* iv_config;

  private: // class methods
    /** @brief schedule a gpio event handler */
    void scheduleGPIOEvent();

    /** @brief handle the GPIO event */
    void handleGPIOEvent();

    /** @brief register for a gpio event */
    void requestGPIOEvent();
};

} // namespace attn
