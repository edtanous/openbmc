#include <array>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <endian.h>
#include <sbe_chipOp_handler.hpp>
#include <file.hpp>
namespace openpower
{
namespace sbe
{
namespace internal
{

constexpr uint16_t MAGIC_CODE = 0xC0DE;
constexpr auto SBE_OPERATION_SUCCESSFUL = 0;
constexpr auto LENGTH_OF_DISTANCE_HEADER_IN_WORDS = 0x1;
constexpr auto LENGTH_OF_RESP_HEADER_IN_WORDS = 0x2;
constexpr auto DISTANCE_TO_RESP_CODE = 0x1;
constexpr auto MAX_FFDC_LEN_IN_WORDS = 5120;
constexpr auto WORD_SIZE = 4;
constexpr auto MAGIC_CODE_BITS = 16;
std::vector<sbe_word_t> writeToFifo(const char* devPath,
                                    const sbe_word_t* cmdBuffer,
                                    size_t cmdBufLen,
                                    size_t respBufLen)
{
    ssize_t len = 0;
    std::vector<sbe_word_t> response;
    std::ostringstream errMsg;

    //Open the device and obtain the file descriptor associated with it.
    FileDescriptor fileFd(devPath, (O_RDWR | O_NONBLOCK));

    //Wait for FIFO device and perform write operation
    struct pollfd poll_fd = {};
    poll_fd.fd = fileFd();
    poll_fd.events = POLLOUT | POLLERR;

    int rc = 0;
    if ((rc = poll(&poll_fd, 1, -1)) < 0)
    {
        //TODO:use elog infrastructure
        errMsg << "Waiting for FIFO device:" << devPath << "to write failed"
               << "rc=" << rc << "errno=" << errno;
        throw std::runtime_error(errMsg.str().c_str());
    }
    if (poll_fd.revents & POLLERR)
    {
        //TODO:use elog infrastructure
        errMsg << "POLLERR while waiting for writeable FIFO,errno:" << errno;
        throw std::runtime_error(errMsg.str().c_str());
    }
    auto bytesToWrite = (cmdBufLen * WORD_SIZE);
    //Perform the write operation
    len = write(fileFd(), cmdBuffer, bytesToWrite);
    if (len < 0)
    {
        //TODO:use elog infrastructure
        errMsg << "Failed to write to FIFO device:" << devPath << " Length "
               "returned= " << len << " errno=" << errno;
        throw std::runtime_error(errMsg.str().c_str());
    }
    //Wait for FIFO device and perform read operation
    poll_fd.fd = fileFd();
    poll_fd.events = POLLIN | POLLERR;
    if ((rc = poll(&poll_fd, 1, -1) < 0))
    {
        //TODO:use elog infrastructure
        errMsg << "Waiting for FIFO device:" << devPath << "to read failed"
               << " rc=" << rc << " and errno=" << errno;
        throw std::runtime_error(errMsg.str().c_str());
    }
    if (poll_fd.revents & POLLERR)
    {
        //TODO:use elog infrastructure
        errMsg << "POLLERR while waiting for readable FIFO,errno:" << errno;
        throw std::runtime_error(errMsg.str().c_str());
    }
    //Derive the total read length which should include the FFDC, which SBE
    //returns in case of failure.
    size_t totalReadLen = respBufLen + MAX_FFDC_LEN_IN_WORDS;
    //Create a temporary buffer
    std::vector<sbe_word_t> buffer(totalReadLen);

    ssize_t bytesToRead = (totalReadLen * WORD_SIZE);
    len = read(fileFd(), buffer.data(), bytesToRead);
    if (len < 0)
    {
        //TODO:use elog infrastructure
        errMsg << "Failed to read the FIFO device:" << devPath << "bytes read ="
               << len << " errno=" << errno;
        throw std::runtime_error(errMsg.str().c_str());
    }
    else if (len != bytesToRead)
    {
        //TODO:use elog infrastructure
        errMsg << "Exepcted " << bytesToRead << " bytes to be read from FIFO"
               " device:" << devPath << ", but bytes read are " << len <<
               " errno= " << errno;
        throw std::runtime_error(errMsg.str().c_str());
    }

    //Extract the valid number of words read.
    for (auto i = 0; i < (rc / WORD_SIZE); ++i)
    {
        response.push_back(be32toh(buffer[i]));
    }

    //Closing of the file descriptor will be handled when the FileDescriptor
    //object will go out of scope.
    return response;
}

void parseResponse(std::vector<sbe_word_t>& sbeDataBuf)
{
    //Number of 32-bit words obtained from the SBE
    size_t lengthObtained = sbeDataBuf.size();

    //Fetch the SBE header and SBE chiop primary and secondary status
    //Last value in the buffer will have the offset for the SBE header
    size_t distanceToStatusHeader =  sbeDataBuf[sbeDataBuf.size() - 1];

    if (lengthObtained < distanceToStatusHeader)
    {
        //TODO:use elog infrastructure
        std::ostringstream errMsg;
        errMsg << "Distance to SBE status header value " <<
               distanceToStatusHeader << " is greater then total length of "
               "response buffer " << lengthObtained;
        throw std::runtime_error(errMsg.str().c_str());
    }

    //Fetch the response header contents
    auto iter = sbeDataBuf.begin();
    std::advance(iter, (lengthObtained - distanceToStatusHeader));

    //First header word will have 2 bytes of MAGIC CODE followed by
    //Command class and command type
    //|  MAGIC BYTES:0xCODE | COMMAND-CLASS | COMMAND-TYPE|
    sbe_word_t l_magicCode = (*iter >> MAGIC_CODE_BITS);

    //Fetch the primary and secondary response code
    std::advance(iter, DISTANCE_TO_RESP_CODE);
    auto l_priSecResp = *iter;

    //Validate the magic code obtained in the response
    if (l_magicCode != MAGIC_CODE)
    {
        //TODO:use elog infrastructure
        std::ostringstream errMsg;
        errMsg << "Invalid MAGIC keyword in the response header (" <<
               l_magicCode << "),expected keyword " << MAGIC_CODE;
        throw std::runtime_error(errMsg.str().c_str());
    }

    //Validate the Primary and Secondary response value
    if (l_priSecResp != SBE_OPERATION_SUCCESSFUL)
    {
        //Extract the SBE FFDC and throw it to the caller
        size_t ffdcLen = (distanceToStatusHeader -
                          LENGTH_OF_RESP_HEADER_IN_WORDS -
                          LENGTH_OF_DISTANCE_HEADER_IN_WORDS);
        if (ffdcLen)
        {
            std::vector<sbe_word_t> ffdcData(ffdcLen);
            //Fetch the offset of FFDC data
            auto ffdcOffset = (lengthObtained - distanceToStatusHeader) +
                              LENGTH_OF_RESP_HEADER_IN_WORDS;
            std::copy_n((sbeDataBuf.begin() + ffdcOffset), ffdcLen,
                        ffdcData.begin());
        }

        //TODO:use elog infrastructure to return the SBE and Hardware procedure
        //FFDC container back to the caller.
        std::ostringstream errMsg;
        errMsg << "Chip operation failed with SBE response code:" << l_priSecResp
               << ".Length of FFDC data of obtained:" << ffdcLen;
        throw std::runtime_error(errMsg.str().c_str());
    }

    //In case of success, remove the response header content and send only the
    //data.Response header will be towards the end of the buffer.
    auto respLen = (lengthObtained - distanceToStatusHeader);
    iter = sbeDataBuf.begin();
    std::advance(iter,respLen);
    sbeDataBuf.erase(iter, sbeDataBuf.end());
}

}
}
}
