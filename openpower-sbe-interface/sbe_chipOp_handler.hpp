#pragma once

#include <stdexcept>
#include <array>
#include <sstream>
#include <algorithm>
#include <vector>

namespace openpower
{
namespace sbe
{

using sbe_word_t = uint32_t;

namespace internal
{

/**
 * @brief Helper function for invokeSBEChipOperation(),to write to the SBE FIFO
 * device and obtain the expected response .Internal device driver failures
 * will be conveyed via respective exceptions.
 *
 * Exceptions thrown for:
 * - Device driver internal failures
 *
 * @param[in] FIFO device path associated with SBE.
 * @param[in] Command buffer to be written to the SBE FIFO
 * @param[in] Length of command buffer
 * @param[in] Expected response buffer length
 *
 * @return Response buffer returned by the SBE for the input command.
 */
std::vector<sbe_word_t> write(const char* devPath,
                              const sbe_word_t* cmdBuffer,
                              size_t cmdBufLen,
                              size_t respBufLen);

/**
 * @brief Helper function for invokeSBEChipOperation(), to parse the data
 * obtained from the SBE. The header and SBE response will be verified and on
 * success the required data will be returned to the caller. SBE interface
 * failure will be conveyed via respective exceptions.
 *
 * Exceptions thrown for:
 * - SBE Internal failures
 *
 * @param[in] SBE data obtained from the SBE FIFO device
 * @return Valid chip operation response obtained by SBE.
 */
std::vector<sbe_word_t> parseResponse(
    const std::vector<sbe_word_t>& sbeDataBuf);

}//end of internal namespace

/**
 * @brief Interface to invoke a SBE chip operation.It calls internal API to
 * write to the SBE FIFO and validates the data obtained by the SBE. It throws
 * exception for any SBE internal failures.
 *
 * Runtime exceptions thrown for:
 * - Device driver failures
 * - SBE internal failures
 *
 * @param[in] FIFO device path associated with the SBE.
 * @param[in] Request packet for the data to be read.
 * @param[in] Data obtained by the SBE.
 * @tparam S1 Length of request buffer to be send to SBE
 * @tparam S2 Expected length of data from the SBE
 */
template<size_t S1, size_t S2>
inline void invokeSBEChipOperation(const char* devPath,
                                   const std::array<sbe_word_t, S1>& request,
                                   std::array<sbe_word_t, S2>& chipOpData)
{
    //Write and read from the FIFO device.
    auto sbeFifoResp = internal::write(devPath, request.data(), request.size(),
                                       chipOpData.size());

    //Parse the obtained data
    auto response = internal::parseResponse(sbeFifoResp);

    if (response.size() != chipOpData.size())
    {
        //TODO:use elog infrastructure
        std::ostringstream errMsg;
        errMsg << "Obtained chip operation response length (" << response.size()
               << "from SBE is not equal to the expected length of data (" <<
               chipOpData.size();

        throw std::runtime_error(errMsg.str().c_str());
    }

    //Move the contents of response buffer into the output buffer.
    std::move(response.begin(), response.end(), chipOpData.begin());
}

}
}

