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

// miscellaneous defines
constexpr uint8_t TI_WITH_PLID = 0x01;
constexpr uint8_t TI_WITH_SRC  = 0x02;
constexpr uint8_t TI_WITH_EID  = 0x03;

// component ID's
constexpr uint16_t INITSVC_COMP_ID = 0x0500;
constexpr uint16_t PNOR_COMP_ID    = 0x0600;
constexpr uint16_t HWAS_COMP_ID    = 0x0C00;
constexpr uint16_t SECURE_COMP_ID  = 0x1E00;
constexpr uint16_t TRBOOT_COMP_ID  = 0x2B00;

// HBFW::INITSERVICE::SHUTDOWNPREQUESTED_BY_FSP
constexpr uint16_t HB_SRC_SHUTDOWN_REQUEST = INITSVC_COMP_ID | 0x0b;

// SHUTDOWN_KEY_TRANSITION
constexpr uint16_t HB_SRC_KEY_TRANSITION = INITSVC_COMP_ID | 0x15;

// HBFW::HWAS::RC_SYSAVAIL_INSUFFICIENT_HW
constexpr uint16_t HB_SRC_INSUFFICIENT_HW = HWAS_COMP_ID | 0x04;

// HBFW::TRUSTDBOOT::RC_TPM_NOFUNCTIONALTPM_FAIL
constexpr uint16_t HB_SRC_TPM_FAIL = TRBOOT_COMP_ID | 0xAD;

// HBFW::SECUREBOOT::RC_ROM_VERIFY
constexpr uint16_t HB_SRC_ROM_VERIFY = SECURE_COMP_ID | 0x07;

// HBFW::PNOR::RC_BASE_EXT_MISMATCH
constexpr uint16_t HB_SRC_EXT_MISMATCH = PNOR_COMP_ID | 0x2F;

// HBFW::PNOR:RC_ECC_UE
constexpr uint16_t HB_SRC_ECC_UE = PNOR_COMP_ID | 0x0F;

// HBFW::PNOR:RC_UNSUPPORTED_MODE
constexpr uint16_t HB_SRC_UNSUPPORTED_MODE = PNOR_COMP_ID | 0x0D;

// HBFW::PNOR:RC_UNSUPPORTED_SFCRANGE
constexpr uint16_t HB_SRC_UNSUPPORTED_SFCRANGE = PNOR_COMP_ID | 0x0E;

// HBFW::PNOR:RC_PARTITION_TABLE_INVALID
constexpr uint16_t HB_SRC_PARTITION_TABLE = PNOR_COMP_ID | 0x0C;

// HBFW::PNOR:RC_UNSUPPORTED_HARDWARE
constexpr uint16_t HB_SRC_UNSUPPORTED_HARDWARE = PNOR_COMP_ID | 0x0A;

// HBFW::PNOR:RC_PNOR_CORRUPTION
constexpr uint16_t HB_SRC_PNOR_CORRUPTION = PNOR_COMP_ID | 0x99;

/** @brief Handle terminate immediate special attentions */
int tiHandler(TiDataArea* i_tiDataArea);

/** @brief Handle phyp terminate immediately special attention */
void handlePhypTi(TiDataArea* i_tiDataArea);

/** @brief Handle hostboot terminate immediately special attention */
void handleHbTi(TiDataArea* i_tiDataArea);

/**
 * @brief Parse TI info data as raw 32-bit fields
 *
 * Read the TI data in as 32-bit fields and place into map.
 */
void parseRawTiInfo(std::map<std::string, std::string>& i_map,
                    TiDataArea* i_buffer);

/**
 * @brief Parse TI info data as PHYP/OPAL data
 *
 * Read the TI data, parse as PHYP/OPAL data and place into map.
 */
void parsePhypOpalTiInfo(std::map<std::string, std::string>& i_map,
                         TiDataArea* i_tiDataArea);

/**
 * @brief Parse TI info data as hostboot data
 *
 * Read the TI data, parse as hostboot data and place into map.
 */
void parseHbTiInfo(std::map<std::string, std::string>& i_map,
                   TiDataArea* i_tiDataArea);

/**
 * @brief Read autoreboot property
 *
 * Read the autoreboot property via dbus. This status will be used to
 * determine whether to either mpipl or quiesce the host on TI condition.
 */
bool autoRebootEnabled();

} // namespace attn
