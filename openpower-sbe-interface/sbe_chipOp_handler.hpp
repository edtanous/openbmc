#pragma once

#include <algorithm>
#include <array>
#include <sbe_interfaces.hpp>
#include <sstream>
#include <stdexcept>
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
std::vector<sbe_word_t> writeToFifo(const char* devPath,
                                    const sbe_word_t* cmdBuffer,
                                    size_t cmdBufLen, size_t respBufLen);

/**
 * @brief Helper function for invokeSBEChipOperation(), to parse and validate
 * the data obtained from the SBE. Input buffer will be validated and on failure
 * the FFDC content will be extracted and returned to the caller via
 * respective exception. On success the input buffer will be modified to have
 * only valid response data after removing the header content.
 *
 * Exceptions thrown for:
 * - SBE Internal failures
 *
 * @param[in/out] On input  - SBE data obtained from the SBE FIFO device.
 *                On output - Chip operation data after removing the response
 *                            header.
 */
void parseResponse(std::vector<sbe_word_t>& sbeDataBuf);

} // namespace internal

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
template <size_t S1, size_t S2>
inline void invokeSBEChipOperation(const char* devPath,
                                   const std::array<sbe_word_t, S1>& request,
                                   std::array<sbe_word_t, S2>& chipOpData)
{
    // Write and read from the FIFO device.
    auto sbeFifoResp = internal::writeToFifo(devPath, request.data(),
                                             request.size(), chipOpData.size());

    // Parse the obtained data
    internal::parseResponse(sbeFifoResp);
    // Above interface would have stripped the SBE header content from the input
    // response buffer.
    if (sbeFifoResp.size() > chipOpData.size())
    {
        // TODO:use elog infrastructure
        std::ostringstream errMsg;
        errMsg << "Obtained chip operation response length ("
               << sbeFifoResp.size()
               << "from SBE is greater than maximum expected"
                  " length:"
               << chipOpData.size();

        throw std::runtime_error(errMsg.str().c_str());
    }

    // Move the contents of response buffer into the output buffer.
    std::move(sbeFifoResp.begin(), sbeFifoResp.end(), chipOpData.begin());
}

} // namespace sbe
} // namespace openpower
