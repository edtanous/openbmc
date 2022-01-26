#include <util/bin_stream.hpp>

#include "gtest/gtest.h"

enum RegisterId_t : uint32_t; // Defined in hei_types.hpp

namespace util
{

/** @brief Extracts big-endian data to host RegisterId_t. */
template <>
inline BinFileReader& BinFileReader::operator>>(RegisterId_t& r)
{
    // A register ID is only 3 bytes, but there isn't a 3-byte integer type.
    // So extract 3 bytes to a uint32_t and drop the unused byte.
    uint32_t tmp = 0;
    read(&tmp, 3);
    r = static_cast<RegisterId_t>(be32toh(tmp) >> 8);
    return *this;
}

/** @brief Inserts host RegisterId_t to big-endian data. */
template <>
inline BinFileWriter& BinFileWriter::operator<<(RegisterId_t r)
{
    // A register ID is only 3 bytes, but there isn't a 3-byte integer type.
    // So extract 3 bytes to a uint32_t and drop the unused byte.
    uint32_t tmp = htobe32(static_cast<uint32_t>(r) << 8);
    write(&tmp, 3);
    return *this;
}

} // namespace util

TEST(BinStream, TestSet1)
{
    uint8_t w1      = 0x11;
    uint16_t w2     = 0x1122;
    uint32_t w3     = 0x11223344;
    uint64_t w4     = 0x1122334455667788;
    RegisterId_t w5 = static_cast<RegisterId_t>(0x123456);

    {
        // need scope so the writer is closed before the reader opens
        util::BinFileWriter w{"bin_stream_test.bin"};
        ASSERT_TRUE(w.good());

        w << w1 << w2 << w3 << w4 << w5;
        ASSERT_TRUE(w.good());
    }

    uint8_t r1;
    uint16_t r2;
    uint32_t r3;
    uint64_t r4;
    RegisterId_t r5;

    {
        util::BinFileReader r{"bin_stream_test.bin"};
        ASSERT_TRUE(r.good());

        r >> r1 >> r2 >> r3 >> r4 >> r5;
        ASSERT_TRUE(r.good());
    }

    ASSERT_EQ(w1, r1);
    ASSERT_EQ(w2, r2);
    ASSERT_EQ(w3, r3);
    ASSERT_EQ(w4, r4);
    ASSERT_EQ(w5, r5);
}
