#pragma once

/**
 * @brief Specially utilities that are specific to the analyzer (typically
 *        stuff that involves libhei).
 */

#include <hei_main.hpp>
#include <util/bin_stream.hpp>

namespace util
{

/** @brief Extracts big-endian data to host RegisterId_t. */
template <>
inline BinFileReader& BinFileReader::operator>>(libhei::RegisterId_t& r)
{
    // A register ID is only 3 bytes, but there isn't a 3-byte integer type.
    // So extract 3 bytes to a uint32_t and drop the unused byte.
    uint32_t tmp = 0;
    read(&tmp, 3);
    r = static_cast<libhei::RegisterId_t>(be32toh(tmp) >> 8);
    return *this;
}

/** @brief Inserts host RegisterId_t to big-endian data. */
template <>
inline BinFileWriter& BinFileWriter::operator<<(libhei::RegisterId_t r)
{
    // A register ID is only 3 bytes, but there isn't a 3-byte integer type.
    // So extract 3 bytes to a uint32_t and drop the unused byte.
    uint32_t tmp = htobe32(static_cast<uint32_t>(r) << 8);
    write(&tmp, 3);
    return *this;
}

} // namespace util
