#include <attn/attn_common.hpp>
#include <attn/attn_dbus.hpp>
#include <attn/attn_dump.hpp>
#include <attn/attn_logging.hpp>
#include <attn/pel/pel_common.hpp>
#include <attn/ti_handler.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <util/dbus.hpp>

#include <iomanip>
#include <iostream>

namespace attn
{

/**
 * @brief Determine if this is a HB or PHYP TI event
 *
 * Use the TI info data area to determine if this is either a HB or a PHYP
 * TI event then handle the event.
 *
 * @param i_tiDataArea pointer to the TI info data
 */
int tiHandler(TiDataArea* i_tiDataArea)
{
    int rc = RC_SUCCESS;

    // capture some additional data for logs/traces
    addHbStatusRegs();

    // check TI data area if it is available
    if (nullptr != i_tiDataArea)
    {
        // HB v. PHYP TI logic: Only hosboot will fill in hbTerminateType
        // and it will be non-zero. Only hostboot will fill out source and
        // it it will be non-zero. Only PHYP will fill in srcFormat and it
        // will be non-zero.
        if ((0 == i_tiDataArea->hbTerminateType) &&
            (0 == i_tiDataArea->source) && (0 != i_tiDataArea->srcFormat))
        {
            handlePhypTi(i_tiDataArea);
        }
        else
        {
            handleHbTi(i_tiDataArea);
        }
    }
    else
    {
        // TI data was not available This should not happen since we provide
        // a default TI info in the case where get TI info was not successful.
        eventAttentionFail((int)AttnSection::tiHandler | ATTN_INFO_NULL);
        rc = RC_NOT_HANDLED;
    }

    return rc;
}

/**
 * @brief Handle a PHYP terminate immediate special attention
 *
 * The TI info data area will contain information pertaining to the TI
 * condition. We will wither quiesce the host or initiate a MPIPL depending
 * depending on the auto reboot configuration. We will also create a PEL which
 * will contain the TI info data and FFDC data captured in the system journal.
 *
 * @param i_tiDataArea pointer to TI information filled in by hostboot
 */
void handlePhypTi(TiDataArea* i_tiDataArea)
{
    trace<level::INFO>("PHYP TI");

    // gather additional data for PEL
    std::map<std::string, std::string> tiAdditionalData;

    // make note of recoverable errors present
    tiAdditionalData["recoverables"] = recoverableErrors() ? "true" : "false";

    if (nullptr != i_tiDataArea)
    {
        parsePhypOpalTiInfo(tiAdditionalData, i_tiDataArea);

        tiAdditionalData["Subsystem"] =
            std::to_string(static_cast<uint8_t>(pel::SubsystemID::hypervisor));

        // Copy all ascii src chars to additional data
        char srcChar[33]; // 32 ascii chars + null term
        memcpy(srcChar, &(i_tiDataArea->asciiData0), 32);
        srcChar[32]                  = 0;
        tiAdditionalData["SrcAscii"] = std::string{srcChar};

        // TI event
        eventTerminate(tiAdditionalData, (char*)i_tiDataArea);
    }
    else
    {
        // TI data was not available This should not happen since we provide
        // a default TI info in the case where get TI info was not successful.
        eventAttentionFail((int)AttnSection::handlePhypTi | ATTN_INFO_NULL);
    }

    // We are finished creating the event log entries so transition host to
    // the required state.
    if (true == util::dbus::dumpPolicyEnabled())
    {
        // MPIPL is considered a "dump" so we will qualify this transition with
        // the dumpPolicyEnabled property. MPIPL is triggered by by starting
        // the host "crash" target.
        util::dbus::transitionHost(util::dbus::HostState::Crash);
    }
    else
    {
        // If dumpPolicyEnabled property is disabled we will quiesce the host
        util::dbus::transitionHost(util::dbus::HostState::Quiesce);
    }
}

/**
 * @brief Handle a hostboot terminate immediate with SRC provided
 *
 * The TI info will contain the log ID of the event log that has already been
 * submitted by hostboot. In this case the attention handler does not need to
 * create a PEL. A hostboot dump may be requested and the host will be
 * transitioned.
 *
 * @param i_tiDataArea pointer to TI information filled in by hostboot
 */
void handleHbTiWithEid(TiDataArea* i_tiDataArea)
{
    trace<level::INFO>("HB TI with PLID/EID");

    if (nullptr != i_tiDataArea)
    {
        // see if HB dump is requested
        if (0 != i_tiDataArea->hbDumpFlag)
        {
            // retrieve log ID from TI info data
            uint32_t logId = be32toh(i_tiDataArea->asciiData1);
            requestDump(logId, DumpParameters{0, DumpType::Hostboot});
        }
    }

    util::dbus::transitionHost(util::dbus::HostState::Quiesce);
}

/**
 * @brief Handle a hostboot terminate immediate with SRC provided
 *
 * The TI info will contain the reason code and additional data necessary
 * to create a PEL on behalf of hostboot. A hostboot dump may be created
 * (after generating the PEL) and the host may be transitioned depending
 * on the reason code.
 *
 * @param i_tiDataArea pointer to TI information filled in by hostboot
 */
void handleHbTiWithSrc(TiDataArea* i_tiDataArea)
{
    trace<level::INFO>("HB TI with SRC");

    // handle specific hostboot reason codes
    if (nullptr != i_tiDataArea)
    {
        // Reason code is byte 2 and 3 of 4 byte srcWord12HbWord0
        uint16_t reasonCode = be32toh(i_tiDataArea->srcWord12HbWord0);

        if (reasonCode != HB_SRC_SHUTDOWN_REQUEST)
        {
            // gather additional data for PEL
            std::map<std::string, std::string> tiAdditionalData;

            // make note of recoverable errors present
            tiAdditionalData["recoverables"] =
                recoverableErrors() ? "true" : "false";

            parseHbTiInfo(tiAdditionalData, i_tiDataArea);

            tiAdditionalData["Subsystem"] = std::to_string(
                static_cast<uint8_t>(pel::SubsystemID::hostboot));

            // Translate hex src value to ascii. This results in an 8
            // character SRC (hostboot SRC is 32 bits)
            std::stringstream src;
            src << std::setw(8) << std::setfill('0') << std::uppercase
                << std::hex << be32toh(i_tiDataArea->srcWord12HbWord0);
            tiAdditionalData["SrcAscii"] = src.str();

            // Request dump after generating event log?
            tiAdditionalData["Dump"] =
                (0 != i_tiDataArea->hbDumpFlag) ? "true" : "false";

            // Generate event log
            eventTerminate(tiAdditionalData, (char*)i_tiDataArea);
        }

        if (HB_SRC_KEY_TRANSITION != reasonCode)
        {
            util::dbus::transitionHost(util::dbus::HostState::Quiesce);
        }
    }
    else
    {
        // TI data was not available, this should not happen
        eventAttentionFail((int)AttnSection::handleHbTi | ATTN_INFO_NULL);
    }
}

/**
 * @brief Handle a hostboot terminate immediate special attention
 *
 * The TI info data area will contain information pertaining to the TI
 * condition. The course of action to take regarding the host state will
 * depend on the contents of the TI info data area. We will also create a
 * PEL containing the TI info data and FFDC data captured in the system
 * journal.
 *
 * @param i_tiDataArea pointer to TI information filled in by hostboot
 */
void handleHbTi(TiDataArea* i_tiDataArea)
{
    trace<level::INFO>("HB TI");

    // handle specific hostboot reason codes
    if (nullptr != i_tiDataArea)
    {
        uint8_t terminateType = i_tiDataArea->hbTerminateType;

        if (TI_WITH_SRC == terminateType)
        {
            handleHbTiWithSrc(i_tiDataArea);
        }
        else
        {
            handleHbTiWithEid(i_tiDataArea);
        }
    }
    else
    {
        // TI data was not available, this should not happen
        eventAttentionFail((int)AttnSection::handleHbTi | ATTN_INFO_NULL);
    }
}

/** @brief Parse the TI info data area into map as PHYP/OPAL data */
void parsePhypOpalTiInfo(std::map<std::string, std::string>& i_map,
                         TiDataArea* i_tiDataArea)
{
    if (nullptr == i_tiDataArea)
    {
        return;
    }

    std::stringstream ss;

    ss << "0x00 TI Area Valid:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->tiAreaValid << ":";
    ss << "0x01 Command:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->command << ":";
    ss << "0x02 Num. Data Bytes:" << std::setw(4) << std::setfill('0')
       << std::hex << be16toh(i_tiDataArea->numDataBytes) << ":";
    ss << "0x04 Reserved:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->reserved1 << ":";
    ss << "0x06 HWDump Type:" << std::setw(4) << std::setfill('0') << std::hex
       << be16toh(i_tiDataArea->hardwareDumpType) << ":";
    ss << "0x08 SRC Format:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->srcFormat << ":";
    ss << "0x09 SRC Flags:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->srcFlags << ":";
    ss << "0x0a Num. ASCII Words:" << std::setw(2) << std::setfill('0')
       << std::hex << (int)i_tiDataArea->numAsciiWords << ":";
    ss << "0x0b Num. Hex Words:" << std::setw(2) << std::setfill('0')
       << std::hex << (int)i_tiDataArea->numHexWords << ":";
    ss << "0x0e Length of SRC:" << std::setw(4) << std::setfill('0') << std::hex
       << be16toh(i_tiDataArea->lenSrc) << ":";
    ss << "0x10 SRC Word 12:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord12HbWord0) << ":";
    ss << "0x14 SRC Word 13:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord13HbWord2) << ":";
    ss << "0x18 SRC Word 14:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord14HbWord3) << ":";
    ss << "0x1c SRC Word 15:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord15HbWord4) << ":";
    ss << "0x20 SRC Word 16:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord16HbWord5) << ":";
    ss << "0x24 SRC Word 17:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord17HbWord6) << ":";
    ss << "0x28 SRC Word 18:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord18HbWord7) << ":";
    ss << "0x2c SRC Word 19:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord19HbWord8) << ":";
    ss << "0x30 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData0) << ":";
    ss << "0x34 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData1) << ":";
    ss << "0x38 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData2) << ":";
    ss << "0x3c ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData3) << ":";
    ss << "0x40 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData4) << ":";
    ss << "0x44 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData5) << ":";
    ss << "0x48 ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData6) << ":";
    ss << "0x4c ASCII Data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData7) << ":";
    ss << "0x50 Location:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->location << ":";
    ss << "0x51 Code Sections:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->codeSection << ":";
    ss << "0x52 Additional Size:" << std::setw(2) << std::setfill('0')
       << std::hex << (int)i_tiDataArea->additionalSize << ":";
    ss << "0x53 Additional Data:" << std::setw(2) << std::setfill('0')
       << std::hex << (int)i_tiDataArea->andData;

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

/** @brief Parse the TI info data area into map as hostboot data */
void parseHbTiInfo(std::map<std::string, std::string>& i_map,
                   TiDataArea* i_tiDataArea)
{
    if (nullptr == i_tiDataArea)
    {
        return;
    }

    std::stringstream ss;

    ss << "0x00 TI Area Valid:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->tiAreaValid << ":";
    ss << "0x04 Reserved:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->reserved1 << ":";
    ss << "0x05 HB_Term. Type:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->hbTerminateType << ":";
    ss << "0x0c HB Dump Flag:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->hbDumpFlag << ":";
    ss << "0x0d Source:" << std::setw(2) << std::setfill('0') << std::hex
       << (int)i_tiDataArea->source << ":";
    ss << "0x10 HB Word 0:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord12HbWord0) << ":";
    ss << "0x14 HB Word 2:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord13HbWord2) << ":";
    ss << "0x18 HB Word 3:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord14HbWord3) << ":";
    ss << "0x1c HB Word 4:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord15HbWord4) << ":";
    ss << "0x20 HB Word 5:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord16HbWord5) << ":";
    ss << "0x24 HB Word 6:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord17HbWord6) << ":";
    ss << "0x28 HB Word 7:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord18HbWord7) << ":";
    ss << "0x2c HB Word 8:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->srcWord19HbWord8) << ":";
    ss << "0x30 error_data:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData0) << ":";
    ss << "0x34 EID:" << std::setw(8) << std::setfill('0') << std::hex
       << be32toh(i_tiDataArea->asciiData1);

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

} // namespace attn
