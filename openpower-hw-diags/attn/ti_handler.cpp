#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <attn/ti_handler.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>

#include <iomanip>
#include <iostream>

namespace attn
{

/** @brief Start host diagnostic mode or quiesce host on TI */
int tiHandler(TiDataArea* i_tiDataArea)
{
    int rc = RC_SUCCESS;

    std::map<std::string, std::string> tiAdditionalData;

    // If TI Info is available we can add it to the PEL
    if (nullptr != i_tiDataArea)
    {
        if (0xa1 == i_tiDataArea->command)
        {
            parsePhypOpalTiInfo(tiAdditionalData, i_tiDataArea);
        }
        else
        {
            parseHbTiInfo(tiAdditionalData, i_tiDataArea);
        }
        parseRawTiInfo(tiAdditionalData, i_tiDataArea);
    }

    eventTerminate(tiAdditionalData); // generate PEL

    // Transition host by starting appropriate dbus target
    auto bus    = sdbusplus::bus::new_system();
    auto method = bus.new_method_call(
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", "StartUnit");

    // If TI info is not available we are going to assume PHYP TI for now
    // (as opposed to HB TI - we have already decided it is not a breakpoint).
    if ((nullptr == i_tiDataArea) || (0xa1 == i_tiDataArea->command))
    {
        trace<level::INFO>("PHYP TI");

        if (autoRebootEnabled())
        {
            // If autoreboot is enabled we will start diagnostic mode target
            // which will ultimately mpipl the host.
            trace<level::INFO>("start obmc-host-diagnostic-mode target");
            method.append("obmc-host-diagnostic-mode@0.target");
        }
        else
        {
            // If autoreboot is disabled we will quiesce the host
            trace<level::INFO>("start obmc-host-quiesce target");
            method.append("obmc-host-quiesce@0.target");
        }
    }
    else
    {
        // For now we are just going to just quiesce host on HB TI
        trace<level::INFO>("HB TI");
        trace<level::INFO>("start obmc-host-quiesce target");
        method.append("obmc-host-quiesce@0.target");
    }

    method.append("replace"); // mode = replace conflicting queued jobs
    bus.call_noreply(method); // start the service

    return rc;
}

/** @brief Parse the TI info data area into map as raw 32-bit fields */
void parseRawTiInfo(std::map<std::string, std::string>& i_map,
                    TiDataArea* i_buffer)
{

    uint32_t* tiDataArea = (uint32_t*)i_buffer;
    std::stringstream ss;

    ss << std::hex << std::setfill('0');
    ss << "raw:";
    while (tiDataArea <= (uint32_t*)((char*)i_buffer + sizeof(TiDataArea)))
    {
        ss << std::setw(8) << std::endl << be32toh(*tiDataArea);
        tiDataArea++;
    }

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

/** @brief Parse the TI info data area into map as PHYP/OPAL data */
void parsePhypOpalTiInfo(std::map<std::string, std::string>& i_map,
                         TiDataArea* i_tiDataArea)
{
    std::stringstream ss;

    ss << std::hex << std::showbase;
    ss << "0x00 TI Area Valid:" << (int)i_tiDataArea->tiAreaValid << ":";
    ss << "0x01 Command:" << (int)i_tiDataArea->command << ":";
    ss << "0x02 Num. Data Bytes:" << be16toh(i_tiDataArea->numDataBytes) << ":";
    ss << "0x04 Reserved:" << (int)i_tiDataArea->reserved1 << ":";
    ss << "0x06 HWDump Type:" << be16toh(i_tiDataArea->hardwareDumpType) << ":";
    ss << "0x08 SRC Format:" << (int)i_tiDataArea->srcFormat << ":";
    ss << "0x09 SRC Flags:" << (int)i_tiDataArea->srcFlags << ":";
    ss << "0x0a Num. ASCII Words:" << (int)i_tiDataArea->numAsciiWords << ":";
    ss << "0x0b Num. Hex Words:" << (int)i_tiDataArea->numHexWords << ":";
    ss << "0x0e Length of SRC:" << be16toh(i_tiDataArea->lenSrc) << ":";
    ss << "0x10 SRC Word 12:" << be32toh(i_tiDataArea->srcWord12HbWord0) << ":";
    ss << "0x14 SRC Word 13:" << be32toh(i_tiDataArea->srcWord13HbWord2) << ":";
    ss << "0x18 SRC Word 14:" << be32toh(i_tiDataArea->srcWord14HbWord3) << ":";
    ss << "0x1c SRC Word 15:" << be32toh(i_tiDataArea->srcWord15HbWord4) << ":";
    ss << "0x20 SRC Word 16:" << be32toh(i_tiDataArea->srcWord16HbWord5) << ":";
    ss << "0x24 SRC Word 17:" << be32toh(i_tiDataArea->srcWord17HbWord6) << ":";
    ss << "0x28 SRC Word 18:" << be32toh(i_tiDataArea->srcWord18HbWord7) << ":";
    ss << "0x2c SRC Word 19:" << be32toh(i_tiDataArea->srcWord19HbWord8) << ":";
    ss << "0x30 ASCII Data:" << be32toh(i_tiDataArea->asciiData0) << ":";
    ss << "0x34 ASCII Data:" << be32toh(i_tiDataArea->asciiData1) << ":";
    ss << "0x38 ASCII Data:" << be32toh(i_tiDataArea->asciiData2) << ":";
    ss << "0x3c ASCII Data:" << be32toh(i_tiDataArea->asciiData3) << ":";
    ss << "0x40 ASCII Data:" << be32toh(i_tiDataArea->asciiData4) << ":";
    ss << "0x44 ASCII Data:" << be32toh(i_tiDataArea->asciiData5) << ":";
    ss << "0x48 ASCII Data:" << be32toh(i_tiDataArea->asciiData6) << ":";
    ss << "0x4c ASCII Data:" << be32toh(i_tiDataArea->asciiData7) << ":";
    ss << "0x50 Location:" << (int)i_tiDataArea->location << ":";
    ss << "0x51 Code Sections:" << (int)i_tiDataArea->codeSection << ":";
    ss << "0x52 Additional Size:" << (int)i_tiDataArea->additionalSize << ":";
    ss << "0x53 Additional Data:" << (int)i_tiDataArea->andData;

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
    std::stringstream ss;

    ss << std::hex << std::showbase;
    ss << "0x00 TI Area Valid:" << (int)i_tiDataArea->tiAreaValid << ":";
    ss << "0x04 Reserved:" << (int)i_tiDataArea->reserved1 << ":";
    ss << "0x05 HB_Term. Type:" << (int)i_tiDataArea->hbTerminateType << ":";
    ss << "0x0c HB Dump Flag:" << (int)i_tiDataArea->hbDumpFlag << ":";
    ss << "0x0d Source:" << (int)i_tiDataArea->source << ":";
    ss << "0x10 HB Word 0:" << be32toh(i_tiDataArea->srcWord12HbWord0) << ":";
    ss << "0x14 HB Word 2:" << be32toh(i_tiDataArea->srcWord13HbWord2) << ":";
    ss << "0x18 HB Word 3:" << be32toh(i_tiDataArea->srcWord14HbWord3) << ":";
    ss << "0x1c HB Word 4:" << be32toh(i_tiDataArea->srcWord15HbWord4) << ":";
    ss << "0x20 HB Word 5:" << be32toh(i_tiDataArea->srcWord16HbWord5) << ":";
    ss << "0x24 HB Word 6:" << be32toh(i_tiDataArea->srcWord17HbWord6) << ":";
    ss << "0x28 HB Word 7:" << be32toh(i_tiDataArea->srcWord18HbWord7) << ":";
    ss << "0x2c HB Word 8:" << be32toh(i_tiDataArea->srcWord19HbWord8) << ":";
    ss << "0x30 error_data:" << be32toh(i_tiDataArea->asciiData0) << ":";
    ss << "0x34 EID:" << be32toh(i_tiDataArea->asciiData1);

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

/** @brief Read state of autoreboot propertyi via dbus */
bool autoRebootEnabled()
{
    // Use dbus get-property interface to read the autoreboot property
    auto bus = sdbusplus::bus::new_system();
    auto method =
        bus.new_method_call("xyz.openbmc_project.Settings",
                            "/xyz/openbmc_project/control/host0/auto_reboot",
                            "org.freedesktop.DBus.Properties", "Get");

    method.append("xyz.openbmc_project.Control.Boot.RebootPolicy",
                  "AutoReboot");

    try
    {
        auto reply = bus.call(method);

        std::variant<bool> result;
        reply.read(result);
        auto autoReboot = std::get<bool>(result);

        if (autoReboot)
        {
            trace<level::INFO>("Auto reboot enabled");
            return true;
        }
        else
        {
            trace<level::INFO>("Auto reboot disabled.");
            return false;
        }
    }
    catch (const sdbusplus::exception::SdBusError& ec)
    {
        std::string traceMessage =
            "Error in AutoReboot Get: " + std::string(ec.what());
        trace<level::INFO>(traceMessage.c_str());
        return false; // assume autoreboot disabled
    }
}

} // namespace attn
