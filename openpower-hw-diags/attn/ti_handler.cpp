#include <fmt/format.h>

#include <attn/attn_common.hpp>
#include <attn/attn_dbus.hpp>
#include <attn/attn_dump.hpp>
#include <attn/attn_logging.hpp>
#include <attn/pel/pel_common.hpp>
#include <attn/ti_handler.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <util/dbus.hpp>
#include <util/trace.hpp>

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
    trace::inf("PHYP TI");

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
    trace::inf("HB TI with PLID/EID");

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
    trace::inf("HB TI with SRC");

    // handle specific hostboot reason codes
    if (nullptr != i_tiDataArea)
    {
        // Reason code is byte 2 and 3 of 4 byte srcWord12HbWord0
        uint16_t reasonCode = be32toh(i_tiDataArea->srcWord12HbWord0);

        trace::inf("reason code %04x", reasonCode);

        // for clean shutdown (reason code 050B) no PEL and no dump
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
            tiAdditionalData["SrcAscii"] =
                fmt::format("{:08X}", be32toh(i_tiDataArea->srcWord12HbWord0));

            // dump flag is only valid for TI with EID (not TI with SRC)
            trace::inf("Ignoring TI info dump flag for HB TI with SRC");
            tiAdditionalData["Dump"] = "true";

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
    trace::inf("HB TI");

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

    i_map["0x00 TI Area Valid"] =
        fmt::format("{:02x}", i_tiDataArea->tiAreaValid);
    i_map["0x01 Command"] = fmt::format("{:02x}", i_tiDataArea->command);
    i_map["0x02 Num. Data Bytes"] =
        fmt::format("{:04x}", be16toh(i_tiDataArea->numDataBytes));
    i_map["0x04 Reserved"] = fmt::format("{:02x}", i_tiDataArea->reserved1);
    i_map["0x06 HWDump Type"] =
        fmt::format("{:04x}", be16toh(i_tiDataArea->hardwareDumpType));
    i_map["0x08 SRC Format"] = fmt::format("{:02x}", i_tiDataArea->srcFormat);
    i_map["0x09 SRC Flags"]  = fmt::format("{:02x}", i_tiDataArea->srcFlags);
    i_map["0x0a Num. ASCII Words"] =
        fmt::format("{:02x}", i_tiDataArea->numAsciiWords);
    i_map["0x0b Num. Hex Words"] =
        fmt::format("{:02x}", i_tiDataArea->numHexWords);
    i_map["0x0e Length of SRC"] =
        fmt::format("{:04x}", be16toh(i_tiDataArea->lenSrc));
    i_map["0x10 SRC Word 12"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord12HbWord0));
    i_map["0x14 SRC Word 13"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord13HbWord2));
    i_map["0x18 SRC Word 14"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord14HbWord3));
    i_map["0x1c SRC Word 15"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord15HbWord4));
    i_map["0x20 SRC Word 16"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord16HbWord5));
    i_map["0x24 SRC Word 17"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord17HbWord6));
    i_map["0x28 SRC Word 18"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord18HbWord7));
    i_map["0x2c SRC Word 19"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord19HbWord8));
    i_map["0x30 ASCII Data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData0));
    i_map["0x34 ASCII Data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData1));
    i_map["0x38 ASCII Data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData2));
    i_map["0x3c ASCII Data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData3));
    i_map["0x40 ASCII Data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData4));
    i_map["0x44 ASCII Data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData5));
    i_map["0x48 ASCII Data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData6));
    i_map["0x4c ASCII Data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData7));
    i_map["0x50 Location"] = fmt::format("{:02x}", i_tiDataArea->location);
    i_map["0x51 Code Sections"] =
        fmt::format("{:02x}", i_tiDataArea->codeSection);
    i_map["0x52 Additional Size"] =
        fmt::format("{:02x}", i_tiDataArea->additionalSize);
    i_map["0x53 Additional Data"] =
        fmt::format("{:02x}", i_tiDataArea->andData);
}

/** @brief Parse the TI info data area into map as hostboot data */
void parseHbTiInfo(std::map<std::string, std::string>& i_map,
                   TiDataArea* i_tiDataArea)
{
    if (nullptr == i_tiDataArea)
    {
        return;
    }

    i_map["0x00 TI Area Valid"] =
        fmt::format("{:02x}", i_tiDataArea->tiAreaValid);
    i_map["0x04 Reserved"] = fmt::format("{:02x}", i_tiDataArea->reserved1);
    i_map["0x05 HB_Term. Type"] =
        fmt::format("{:02x}", i_tiDataArea->hbTerminateType);
    i_map["0x0c HB Dump Flag"] =
        fmt::format("{:02x}", i_tiDataArea->hbDumpFlag);
    i_map["0x0d Source"] = fmt::format("{:02x}", i_tiDataArea->source);
    i_map["0x10 HB Word 0"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord12HbWord0));
    i_map["0x14 HB Word 2"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord13HbWord2));
    i_map["0x18 HB Word 3"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord14HbWord3));
    i_map["0x1c HB Word 4"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord15HbWord4));
    i_map["0x20 HB Word 5"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord16HbWord5));
    i_map["0x24 HB Word 6"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord17HbWord6));
    i_map["0x28 HB Word 7"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord18HbWord7));
    i_map["0x2c HB Word 8"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->srcWord19HbWord8));
    i_map["0x30 error_data"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData0));
    i_map["0x34 EID"] =
        fmt::format("{:08x}", be32toh(i_tiDataArea->asciiData1));
}

} // namespace attn
