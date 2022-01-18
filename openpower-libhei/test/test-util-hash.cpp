#include <hei_util.hpp>

#include "gtest/gtest.h"

using namespace libhei;

TEST(UtilHash, TestSet1)
{
    std::string s{"SOME_RANDOM_STRING"};

    uint8_t h1      = 0xBE;
    uint16_t h2     = 0xF2DD;
    uint32_t h3     = 0xDFB31440;
    uint64_t h4     = 0xD0D3828Ec49F687C;
    NodeId_t h5     = 0xF2DD;                              // 2-byte field
    RegisterId_t h6 = static_cast<RegisterId_t>(0x31D080); // 3-byte field

    EXPECT_EQ(h1, hash<uint8_t>(s));
    EXPECT_EQ(h2, hash<uint16_t>(s));
    EXPECT_EQ(h3, hash<uint32_t>(s));
    EXPECT_EQ(h4, hash<uint64_t>(s));
    EXPECT_EQ(h5, hash<NodeId_t>(s));
    EXPECT_EQ(h6, hash<RegisterId_t>(s));
}
