#include <stdexcept>
#include <array>
#include <sbe_chipOp_handler.hpp>
namespace openpower
{
namespace sbe
{
namespace internal
{
/*! \union sbeRespHeader_t
 *  Defines the breakup of the SBE FIFO chip operation response header.
 */
union sbeRespHeader_t
{
    uint32_t commandWord; /**<Entire 32 bit command word */
    struct
    {
        uint8_t command: 8; /**<command value for which response is obtained */
        uint8_t commandClass: 8; /**< Class of the command */
        uint16_t magic: 16; /**< Magic code obtained in the response */
    };
} __attribute__((packed));


constexpr uint16_t MAGIC_CODE = 0xC0DE;
constexpr auto SBE_OPERATION_SUCCESSFUL = 0;
constexpr auto LENGTH_OF_DISTANCE_HEADER_IN_WORDS = 0x1;
constexpr auto LENGTH_OF_RESP_HEADER_IN_WORDS = 0x2;
constexpr auto SBEI_SBE_RESPONSE_SIZE_IN_WORDS = ((sizeof(sbeRespHeader_t) + \
        sizeof(sbe_word_t)) / 4);
constexpr auto DISTANCE_TO_RESP_CODE = 0x1;

std::vector<sbe_word_t> write(const char* devPath,
                              const sbe_word_t* cmdBuffer,
                              size_t cmdBufLen,
                              size_t respBufLen)
{
    //TODO: Add support for reading and writing from the FIFO device
    std::vector<sbe_word_t> response(respBufLen);
    return response;
}

std::vector<sbe_word_t> parseResponse(
    const std::vector<sbe_word_t>& sbeDataBuf)
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
               distanceToStatusHeader << " is greater then total lenght of "
               "response buffer " << lengthObtained;
        throw std::runtime_error(errMsg.str().c_str());
    }

    //Fetch the response header contents
    sbeRespHeader_t l_respHeader{};
    auto iter = sbeDataBuf.begin();
    std::advance(iter, (lengthObtained - distanceToStatusHeader));
    l_respHeader.commandWord = *iter;

    //Fetch the primary and secondary response code
    std::advance(iter, DISTANCE_TO_RESP_CODE);
    auto l_priSecResp = *iter;

    //Validate the magic code obtained in the response
    if (l_respHeader.magic != MAGIC_CODE)
    {
        //TODO:use elog infrastructure
        std::ostringstream errMsg;
        errMsg << "Invalid MAGIC keyword in the response header (" <<
               l_respHeader.magic << "),expected keyword " << MAGIC_CODE;
        throw std::runtime_error(errMsg.str().c_str());
    }

    //Validate the Primary and Secondary response value
    if (l_priSecResp != SBE_OPERATION_SUCCESSFUL)
    {
        //Extract the SBE FFDC and throw it to the caller
        size_t ffdcLen = (distanceToStatusHeader -
                          SBEI_SBE_RESPONSE_SIZE_IN_WORDS -
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
    return sbeDataBuf;

}

}
}
}


