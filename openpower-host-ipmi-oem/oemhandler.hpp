#pragma once

#include <host-ipmid/ipmid-api.h>
#include <stdint.h>

#include <map>
#include <string>

// IPMI commands for net functions.
enum ipmi_netfn_oem_cmds
{
    IPMI_CMD_PREP_FW_UPDATE = 0x10,
    IPMI_CMD_RESET_BMC_AUTH = 0x11,
    IPMI_CMD_PESEL = 0xF0,
    IPMI_CMD_OCC_RESET = 0x0E,
};

/** @brief Read eSEL data into a string
 *
 *  @param[in] filename - filename of file containing eSEL
 *
 *  @return On success return the eSEL data
 */
std::string readESEL(const char* filename);

/** @brief Create OCC metrics log entry
 *
 *  @param[in] eSELData - eSEL data containing OCC metrics data
 */
void createLogEntry(const std::string& eSELData);

ipmi_ret_t ipmi_ibm_oem_partial_esel(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                     ipmi_request_t request,
                                     ipmi_response_t response,
                                     ipmi_data_len_t data_len,
                                     ipmi_context_t context);

struct esel_request_t
{
    uint16_t resid;
    uint16_t selrecord;
    uint16_t offset;
    uint8_t progress;
} __attribute__((packed));

/** @struct SELEventRecord
 *
 *  IPMI SEL event record format.
 */
struct SELEventRecord
{
    uint16_t recordID;        //!< Record ID.
    uint8_t recordType;       //!< Record Type.
    uint32_t timeStamp;       //!< Timestamp.
    uint16_t generatorID;     //!< Generator ID.
    uint8_t eventMsgRevision; //!< Event Message Revision.
    uint8_t sensorType;       //!< Sensor Type.
    uint8_t sensorNum;        //!< Sensor Number.
    uint8_t eventType;        //!< Event Dir | Event Type.
    uint8_t eventData1;       //!< Event Data 1.
    uint8_t eventData2;       //!< Event Data 2.
    uint8_t eventData3;       //!< Event Data 3.
} __attribute__((packed));

using Id = uint8_t;
using Type = uint8_t;
using ReadingType = uint8_t;
using Offset = uint8_t;
using Path = std::string;

struct Data
{
    Id sensorID;
    Type sensorType;
    ReadingType eventReadingType;
    Offset eventOffset;
};

using ObjectIDMap = std::map<Path, Data>;
