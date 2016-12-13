#include <algorithm>
#include <iomanip>

#include "slp.hpp"
#include "slp_meta.hpp"
#include "slp_server.hpp"
#include "sock_channel.hpp"


/* Call Back for the sd event loop */
static int requestHandler(sd_event_source* es, int fd, uint32_t revents,
                          void* userdata)
{
    int rc = slp::SUCCESS;
    timeval tv{slp::TIMEOUT, 0};
    udpsocket::Channel channel(fd, tv);
    std::vector<uint8_t> recvBuff;
    slp::Message req;
    std::vector<uint8_t> resp;
    // Read the packet
    std::tie(rc, recvBuff) = channel.read();

    if (rc < 0)
    {
        std::cerr << "E> SLP Error in Read : " << std::hex << rc << "\n";
        return rc;
    }

    switch (recvBuff[0])
    {
        case slp::VERSION_2:
        {
            std::cout << "SLP Request" << "\n";
            //print the buffer
            std::for_each(recvBuff.begin(), recvBuff.end(),
                          [](uint8_t & ch)
            {
                std::cout << std::hex << std::setfill('0')
                          << std::setw(2) << (int)ch << ' ' ;
            });
            std::cout << "\n";
            //Parse the buffer and construct the req object
            std::tie(rc, req) = slp::parser::parseBuffer(recvBuff);
            if (!rc)
            {
                //Passing the req object to handler to serve it
                std::tie(rc, resp) = slp::handler::processRequest(req);
            }
            break;
        }
        default:
            rc = static_cast<uint8_t>(slp::Error::VER_NOT_SUPPORTED);
            break;
    }

    //if there was error during Parsing of request
    //or processing of request then handle the error.
    if (rc)
    {
        std::cerr << "E> SLP Error rc=" << rc << "\n";
        resp = slp::handler::processError(req, rc);
    }

    //print and send the response
    std::cout << "SLP Response" << "\n";
    std::for_each(resp.begin(), resp.end(),
                  [](uint8_t & ch)
    {
        std::cout << std::hex << std::setfill('0')
                  << std::setw(2) << (int)ch << ' ' ;
    });

    channel.write(resp);
    return slp::SUCCESS;
}


int main(int argc, char* argv[])
{
    slp::udp::Server svr(slp::PORT, requestHandler);
    return svr.run();
}
