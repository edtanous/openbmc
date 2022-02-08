#include <attn/ti_handler.hpp>
#include <util/trace.hpp>

#include <map>
#include <string>

#include "gtest/gtest.h"

namespace
{
uint8_t tiInfo[0x58] = {
    0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41,
    0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
    0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
    0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b};
} // namespace

TEST(TiHandler, TestSet1)
{
    trace::inf("TestSet1: will test ti handler.");

    std::map<std::string, std::string> tiMap;
    attn::TiDataArea* tiData = (attn::TiDataArea*)tiInfo;

    trace::inf("Set values in tiData to the map by parsePhypOpalTiInfo().");
    attn::parsePhypOpalTiInfo(tiMap, tiData);

    // tiAreaValid, offset: 0x00, size: 1, 0x2c
    EXPECT_EQ(tiMap["0x00 TI Area Valid"], "2c");
    // command, offset: 0x01, size: 1, 0x2d
    EXPECT_EQ(tiMap["0x01 Command"], "2d");
    // numDataBytes, offset: 0x02, size: 2, 0x2e 0x2f
    EXPECT_EQ(tiMap["0x02 Num. Data Bytes"], "2e2f");
    // reserved1, offset: 0x04, size: 1, 0x30
    EXPECT_EQ(tiMap["0x04 Reserved"], "30");
    // hardwareDumpType, offset: 0x06, size: 2, 0x32 0x33
    EXPECT_EQ(tiMap["0x06 HWDump Type"], "3233");
    // srcFormat, offset: 0x08, size: 1, 0x34
    EXPECT_EQ(tiMap["0x08 SRC Format"], "34");
    // srcFlags, offset: 0x09, size: 1, 0x35
    EXPECT_EQ(tiMap["0x09 SRC Flags"], "35");
    // numAsciiWords, offset: 0x0a, size: 1, 0x36
    EXPECT_EQ(tiMap["0x0a Num. ASCII Words"], "36");
    // numHexWords, offset: 0x0b, size: 1, 0x37
    EXPECT_EQ(tiMap["0x0b Num. Hex Words"], "37");
    // lenSrc, offset: 0x0e, size: 2, 0x3a 0x3b
    EXPECT_EQ(tiMap["0x0e Length of SRC"], "3a3b");
    // srcWord12HbWord0, offset: 0x10, size: 4, 0x3c 0x3d 0x3e 0x3f
    EXPECT_EQ(tiMap["0x10 SRC Word 12"], "3c3d3e3f");
    // srcWord13HbWord2, offset: 0x14, size: 4, 0x40 0x41 0x42 0x43
    EXPECT_EQ(tiMap["0x14 SRC Word 13"], "40414243");
    // srcWord14HbWord3, offset: 0x18, size: 4, 0x44 0x45 0x46 0x47
    EXPECT_EQ(tiMap["0x18 SRC Word 14"], "44454647");
    // srcWord15HbWord4, offset: 0x1c, size: 4, 0x48 0x49 0x4a 0x4b
    EXPECT_EQ(tiMap["0x1c SRC Word 15"], "48494a4b");
    // srcWord16HbWord5, offset: 0x20, size: 4, 0x4c 0x4d 0x4e 0x4f
    EXPECT_EQ(tiMap["0x20 SRC Word 16"], "4c4d4e4f");
    // srcWord17HbWord6, offset: 0x24, size: 4, 0x50 0x51 0x52 0x53
    EXPECT_EQ(tiMap["0x24 SRC Word 17"], "50515253");
    // srcWord18HbWord7, offset: 0x28, size: 4, 0x54 0x55 0x56 0x57
    EXPECT_EQ(tiMap["0x28 SRC Word 18"], "54555657");
    // srcWord19HbWord8, offset: 0x2c, size: 4, 0x00 0x01 0x02 0x03
    EXPECT_EQ(tiMap["0x2c SRC Word 19"], "00010203");
    // asciiData0, offset: 0x30, size: 4, 0x04 0x05 0x06 0x07
    EXPECT_EQ(tiMap["0x30 ASCII Data"], "04050607");
    // asciiData1, offset: 0x34, size: 4, 0x08 0x09 0x0a 0x0b
    EXPECT_EQ(tiMap["0x34 ASCII Data"], "08090a0b");
    // asciiData2, offset: 0x38, size: 4, 0x0c 0x0d 0x0e 0x0f
    EXPECT_EQ(tiMap["0x38 ASCII Data"], "0c0d0e0f");
    // asciiData3, offset: 0x3c, size: 4, 0x10 0x11 0x12 0x13
    EXPECT_EQ(tiMap["0x3c ASCII Data"], "10111213");
    // asciiData4, offset: 0x40, size: 4, 0x14 0x15 0x16 0x17
    EXPECT_EQ(tiMap["0x40 ASCII Data"], "14151617");
    // asciiData5, offset: 0x44, size: 4, 0x18 0x19 0x1a 0x1b
    EXPECT_EQ(tiMap["0x44 ASCII Data"], "18191a1b");
    // asciiData6, offset: 0x48, size: 4, 0x1c 0x1d 0x1e 0x1f
    EXPECT_EQ(tiMap["0x48 ASCII Data"], "1c1d1e1f");
    // asciiData7, offset: 0x4c, size: 4, 0x20 0x21 0x22 0x23
    EXPECT_EQ(tiMap["0x4c ASCII Data"], "20212223");
    // location, offset: 0x50, size: 1, 0x24
    EXPECT_EQ(tiMap["0x50 Location"], "24");
    // codeSection, offset: 0x51, size: 1, 0x25
    EXPECT_EQ(tiMap["0x51 Code Sections"], "25");
    // additionalSize, offset: 0x52, size: 1, 0x26
    EXPECT_EQ(tiMap["0x52 Additional Size"], "26");
    // andData, offset: 0x53, size: 1, 0x27
    EXPECT_EQ(tiMap["0x53 Additional Data"], "27");
}

TEST(TiHandler, TestSet2)
{
    trace::inf("TestSet2: will test ti handler.");

    attn::TiDataArea* tiData = nullptr;
    std::map<std::string, std::string> tiMap;

    // Use the pre-defined array as test data.
    tiData = (attn::TiDataArea*)tiInfo;

    trace::inf("Set values in tiData to the map by defaultHbTiInfo().");
    attn::parseHbTiInfo(tiMap, tiData);

    // tiAreaValid, offset: 0x00, size: 1, 0x2c
    EXPECT_EQ(tiMap["0x00 TI Area Valid"], "2c");
    // reserved1, offset: 0x04, size: 1, 0x30
    EXPECT_EQ(tiMap["0x04 Reserved"], "30");
    // hbTerminateType, offset: 0x05, size: 1, 0x31
    EXPECT_EQ(tiMap["0x05 HB_Term. Type"], "31");
    // hbDumpFlag, offset: 0x0c, size: 1, 0x38
    EXPECT_EQ(tiMap["0x0c HB Dump Flag"], "38");
    // source, offset: 0x0d, size: 1, 0x39
    EXPECT_EQ(tiMap["0x0d Source"], "39");
    // srcWord12HbWord0, offset: 0x10, size: 4, 0x3c 0x3d 0x3e 0x3f
    EXPECT_EQ(tiMap["0x10 HB Word 0"], "3c3d3e3f");
    // srcWord13HbWord2, offset: 0x14, size: 4, 0x40 0x41 0x42 0x43
    EXPECT_EQ(tiMap["0x14 HB Word 2"], "40414243");
    // srcWord14HbWord3, offset: 0x18, size: 4, 0x44 0x45 0x46 0x47
    EXPECT_EQ(tiMap["0x18 HB Word 3"], "44454647");
    // srcWord15HbWord4, offset: 0x1c, size: 4, 0x48 0x49 0x4a 0x4b
    EXPECT_EQ(tiMap["0x1c HB Word 4"], "48494a4b");
    // srcWord16HbWord5, offset: 0x20, size: 4, 0x4c 0x4d 0x4e 0x4f
    EXPECT_EQ(tiMap["0x20 HB Word 5"], "4c4d4e4f");
    // srcWord17HbWord6, offset: 0x24, size: 4, 0x50 0x51 0x52 0x53
    EXPECT_EQ(tiMap["0x24 HB Word 6"], "50515253");
    // srcWord18HbWord7, offset: 0x28, size: 4, 0x54 0x55 0x56 0x57
    EXPECT_EQ(tiMap["0x28 HB Word 7"], "54555657");
    // srcWord19HbWord8, offset: 0x2c, size: 4, 0x00 0x01 0x02 0x03
    EXPECT_EQ(tiMap["0x2c HB Word 8"], "00010203");
    // asciiData0, offset: 0x30, size: 4, 0x04 0x05 0x06 0x07
    EXPECT_EQ(tiMap["0x30 error_data"], "04050607");
    // asciiData1, offset: 0x34, size: 4, 0x08 0x09 0x0a 0x0b
    EXPECT_EQ(tiMap["0x34 EID"], "08090a0b");
}
