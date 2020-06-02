#pragma once

#include <stdint.h>

/**
 * @brief TI special attention handler
 *
 * Handle special attention due to a terminate immediately (TI) condition.
 */
namespace attn
{

// TI data area definition
#pragma pack(push)
#pragma pack(1)
struct TiDataArea
{
    uint8_t tiAreaValid;       // 0x00, common (non-zero == valid)
    uint8_t command;           // 0x01, phyp/opal = 0xA1
    uint16_t numDataBytes;     // 0x02, phyp/opal
    uint8_t reserved1;         // 0x04, reserved
    uint8_t hbTerminateType;   // 0x05, hostboot only
    uint16_t hardwareDumpType; // 0x06, phyp/opal
    uint8_t srcFormat;         // 0x08, phyp/opal = 0x02
    uint8_t srcFlags;          // 0x09, phyp/opal
    uint8_t numAsciiWords;     // 0x0a, phyp/opal
    uint8_t numHexWords;       // 0x0b, phyp/opal
    uint8_t hbDumpFlag;        // 0x0c, hostboot only
    uint8_t source;            // 0x0d, hostboot only
    uint16_t lenSrc;           // 0x0e, phyp/opal
    uint32_t srcWord12HbWord0; // 0x10, common
    uint32_t srcWord13HbWord2; // 0x14, common (Word1 intentionally skipped)
    uint32_t srcWord14HbWord3; // 0x18, common
    uint32_t srcWord15HbWord4; // 0x1c, common
    uint32_t srcWord16HbWord5; // 0x20, common
    uint32_t srcWord17HbWord6; // 0x24, common
    uint32_t srcWord18HbWord7; // 0x28, common
    uint32_t srcWord19HbWord8; // 0x2c, common
    uint32_t asciiData0;       // 0x30, phyp/opal, hostboot error_data
    uint32_t asciiData1;       // 0x34, phyp/opal, hostboot EID
    uint32_t asciiData2;       // 0x38, phyp/opal
    uint32_t asciiData3;       // 0x3c, phyp/opal
    uint32_t asciiData4;       // 0x40, phyp/opal
    uint32_t asciiData5;       // 0x44, phyp/opal
    uint32_t asciiData6;       // 0x48, phyp/opal
    uint32_t asciiData7;       // 0x4c, phyp/opal
    uint8_t location;          // 0x50, phyp/opal
    uint8_t codeSection;       // 0x51, phyp/opal
    uint8_t additionalSize;    // 0x52, phyp/opal
    uint8_t andData;           // 0x53, phyp/opal
};
#pragma pack(pop)

int tiHandler(TiDataArea* i_tiDataArea);

} // namespace attn
