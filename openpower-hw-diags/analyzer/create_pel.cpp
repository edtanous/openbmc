#include <unistd.h>

#include <analyzer/service_data.hpp>
#include <analyzer/util.hpp>
#include <hei_main.hpp>
#include <phosphor-logging/elog.hpp>
#include <sdbusplus/bus.hpp>
#include <util/bin_stream.hpp>
#include <util/dbus.hpp>
#include <util/ffdc_file.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>
#include <xyz/openbmc_project/Logging/Create/server.hpp>
#include <xyz/openbmc_project/Logging/Entry/server.hpp>

#include <fstream>
#include <memory>

namespace LogSvr = sdbusplus::xyz::openbmc_project::Logging::server;

namespace analyzer
{

//------------------------------------------------------------------------------

enum FfdcSubType_t : uint8_t
{
    FFDC_SIGNATURES      = 0x01,
    FFDC_REGISTER_DUMP   = 0x02,
    FFDC_CALLOUT_FFDC    = 0x03,
    FFDC_HB_SCRATCH_REGS = 0x04,

    // For the callout section, the value of '0xCA' is required per the
    // phosphor-logging openpower-pel extention spec.
    FFDC_CALLOUTS = 0xCA,
};

enum FfdcVersion_t : uint8_t
{
    FFDC_VERSION1 = 0x01,
};

//------------------------------------------------------------------------------

void __getSrc(const libhei::Signature& i_signature, uint32_t& o_word6,
              uint32_t& o_word7, uint32_t& o_word8)
{
    // [ 0:15] chip model
    // [16:23] reserved space in chip ID
    // [24:31] chip EC level
    o_word6 = i_signature.getChip().getType();

    // [ 0:15] chip position
    // [16:23] node position
    // [24:31] signature attention type
    auto chipPos    = util::pdbg::getChipPos(i_signature.getChip());
    uint8_t nodePos = 0; // TODO: multi-node support
    auto attn       = i_signature.getAttnType();

    o_word7 = (chipPos & 0xffff) << 16 | (nodePos & 0xff) << 8 | (attn & 0xff);

    // [ 0:15] signature ID
    // [16:23] signature instance
    // [24:31] signature bit position
    o_word8 = i_signature.toUint32();

    // Word 9 is currently unused
}

//------------------------------------------------------------------------------

void __setSrc(const libhei::Signature& i_rootCause,
              std::map<std::string, std::string>& io_logData)
{
    uint32_t word6 = 0, word7 = 0, word8 = 0;
    __getSrc(i_rootCause, word6, word7, word8);

    io_logData["SRC6"] = std::to_string(word6);
    io_logData["SRC7"] = std::to_string(word7);
    io_logData["SRC8"] = std::to_string(word8);
}

//------------------------------------------------------------------------------

void __addCalloutList(const ServiceData& i_servData,
                      std::vector<util::FFDCFile>& io_userDataFiles)
{
    // Create a new entry for the user data section containing the callout list.
    io_userDataFiles.emplace_back(util::FFDCFormat::JSON, FFDC_CALLOUTS,
                                  FFDC_VERSION1);

    // Use a file stream to write the JSON to file.
    std::ofstream o{io_userDataFiles.back().getPath()};
    o << i_servData.getCalloutList();
}

//------------------------------------------------------------------------------

void __addCalloutFFDC(const ServiceData& i_servData,
                      std::vector<util::FFDCFile>& io_userDataFiles)
{
    // Create a new entry for the user data section containing the FFDC.
    io_userDataFiles.emplace_back(util::FFDCFormat::Custom, FFDC_CALLOUT_FFDC,
                                  FFDC_VERSION1);

    // Use a file stream to write the JSON to file.
    std::ofstream o{io_userDataFiles.back().getPath()};
    o << i_servData.getCalloutFFDC();
}

//------------------------------------------------------------------------------

void __captureSignatureList(const libhei::IsolationData& i_isoData,
                            std::vector<util::FFDCFile>& io_userDataFiles)
{
    // Create a new entry for this user data section regardless if there are any
    // signatures in the list.
    io_userDataFiles.emplace_back(util::FFDCFormat::Custom, FFDC_SIGNATURES,
                                  FFDC_VERSION1);

    // Create a streamer for easy writing to the FFDC file.
    auto path = io_userDataFiles.back().getPath();
    util::BinFileWriter stream{path};

    // The first 4 bytes in the FFDC contains the number of signatures in the
    // list. Then, the list of signatures will follow.

    auto list = i_isoData.getSignatureList();

    uint32_t numSigs = list.size();
    stream << numSigs;

    for (const auto& sig : list)
    {
        // Each signature will use the same format as the SRC (12 bytes each).
        uint32_t word6 = 0, word7 = 0, word8 = 0;
        __getSrc(sig, word6, word7, word8);
        stream << word6 << word7 << word8;
    }

    // If the stream failed for any reason, remove the FFDC file.
    if (!stream.good())
    {
        trace::err("Unable to write signature list FFDC file: %s",
                   path.string().c_str());
        io_userDataFiles.pop_back();
    }
}

//------------------------------------------------------------------------------

void __captureRegisterDump(const libhei::IsolationData& i_isoData,
                           std::vector<util::FFDCFile>& io_userDataFiles)
{
    // Create a new entry for this user data section regardless if there are any
    // registers in the dump.
    io_userDataFiles.emplace_back(util::FFDCFormat::Custom, FFDC_REGISTER_DUMP,
                                  FFDC_VERSION1);

    // Create a streamer for easy writing to the FFDC file.
    auto path = io_userDataFiles.back().getPath();
    util::BinFileWriter stream{path};

    // The first 4 bytes in the FFDC contains the number of chips with register
    // data. Then the data for each chip will follow.

    auto dump = i_isoData.getRegisterDump();

    uint32_t numChips = dump.size();
    stream << numChips;

    for (const auto& entry : dump)
    {
        auto chip    = entry.first;
        auto regList = entry.second;

        // Each chip will have the following information:
        //   4 byte chip model/EC
        //   2 byte chip position
        //   1 byte node position
        //   4 byte number of registers
        // Then the data for each register will follow.

        uint32_t chipType = chip.getType();
        uint16_t chipPos  = util::pdbg::getChipPos(chip);
        uint8_t nodePos   = 0; // TODO: multi-node support
        uint32_t numRegs  = regList.size();
        stream << chipType << chipPos << nodePos << numRegs;

        for (const auto& reg : regList)
        {
            // Each register will have the following information:
            //   3 byte register ID
            //   1 byte register instance
            //   1 byte data size
            //   * byte data buffer (* depends on value of data size)

            libhei::RegisterId_t regId = reg.regId;   // 3 byte
            libhei::Instance_t regInst = reg.regInst; // 1 byte

            auto tmp = libhei::BitString::getMinBytes(reg.data->getBitLen());
            if (255 < tmp)
            {
                trace::inf("Register data execeeded 255 and was truncated: "
                           "regId=0x%06x regInst=%u",
                           regId, regInst);
                tmp = 255;
            }
            uint8_t dataSize = tmp;

            stream << regId << regInst << dataSize;

            stream.write(reg.data->getBufAddr(), dataSize);
        }
    }

    // If the stream failed for any reason, remove the FFDC file.
    if (!stream.good())
    {
        trace::err("Unable to write register dump FFDC file: %s",
                   path.string().c_str());
        io_userDataFiles.pop_back();
    }
}

//------------------------------------------------------------------------------

void __captureHostbootScratchRegisters(
    std::vector<util::FFDCFile>& io_userDataFiles)
{
    // Get the Hostboot scratch registers from the primary processor.

    uint32_t cfamAddr  = 0x283C;
    uint32_t cfamValue = 0;

    uint64_t scomAddr  = 0x4602F489;
    uint64_t scomValue = 0;

    auto priProc = util::pdbg::getPrimaryProcessor();
    if (nullptr == priProc)
    {
        trace::err("Unable to get primary processor");
    }
    else
    {
        if (0 != util::pdbg::getCfam(priProc, cfamAddr, cfamValue))
        {
            cfamValue = 0; // just in case
        }

        if (0 != util::pdbg::getScom(priProc, scomAddr, scomValue))
        {
            scomValue = 0; // just in case
        }
    }

    // Create a new entry for this user data section.
    io_userDataFiles.emplace_back(util::FFDCFormat::Custom,
                                  FFDC_HB_SCRATCH_REGS, FFDC_VERSION1);

    // Create a streamer for easy writing to the FFDC file.
    auto path = io_userDataFiles.back().getPath();
    util::BinFileWriter stream{path};

    // Add the data (CFAM addr/val, then SCOM addr/val).
    stream << cfamAddr << cfamValue << scomAddr << scomValue;

    // If the stream failed for any reason, remove the FFDC file.
    if (!stream.good())
    {
        trace::err("Unable to write register dump FFDC file: %s",
                   path.string().c_str());
        io_userDataFiles.pop_back();
    }
}

//------------------------------------------------------------------------------

std::string __getMessageRegistry(AnalysisType i_type)
{

    if (AnalysisType::SYSTEM_CHECKSTOP == i_type)
    {
        return "org.open_power.HwDiags.Error.Checkstop";
    }
    else if (AnalysisType::TERMINATE_IMMEDIATE == i_type)
    {
        return "org.open_power.HwDiags.Error.Predictive";
    }

    return "org.open_power.HwDiags.Error.Informational"; // default
}

//------------------------------------------------------------------------------

std::string __getMessageSeverity(AnalysisType i_type)
{
    // Default severity is informational (no service action required).
    LogSvr::Entry::Level severity = LogSvr::Entry::Level::Informational;

    if (AnalysisType::SYSTEM_CHECKSTOP == i_type)
    {
        // System checkstops are always unrecoverable errors (service action
        // required).
        severity = LogSvr::Entry::Level::Error;
    }
    else if (AnalysisType::TERMINATE_IMMEDIATE == i_type)
    {
        // TIs will be reported as a predicive error (service action required).
        severity = LogSvr::Entry::Level::Warning;
    }

    // Convert the message severity to a string.
    return LogSvr::Entry::convertLevelToString(severity);
}

//------------------------------------------------------------------------------

uint32_t createPel(const libhei::IsolationData& i_isoData,
                   const ServiceData& i_servData)
{
    uint32_t o_plid = 0; // default, zero indicates PEL was not created

    // The message registry will require additional log data to fill in keywords
    // and additional log data.
    std::map<std::string, std::string> logData;

    // Keep track of the temporary files associated with the user data FFDC.
    // WARNING: Once the objects stored in this vector go out of scope, the
    //          temporary files will be deleted. So they must remain in scope
    //          until the PEL is submitted.
    std::vector<util::FFDCFile> userDataFiles;

    // Set words 6-9 of the SRC.
    __setSrc(i_servData.getRootCause(), logData);

    // Add the list of callouts to the PEL.
    __addCalloutList(i_servData, userDataFiles);

    // Add the Hostboot scratch register to the PEL.
    __captureHostbootScratchRegisters(userDataFiles);

    // Add the callout FFDC to the PEL.
    __addCalloutFFDC(i_servData, userDataFiles);

    // Capture the complete signature list.
    __captureSignatureList(i_isoData, userDataFiles);

    // Capture the complete signature list.
    __captureRegisterDump(i_isoData, userDataFiles);

    // Now, that all of the user data files have been created, transform the
    // data into the proper format for the PEL.
    std::vector<util::FFDCTuple> userData;
    util::transformFFDC(userDataFiles, userData);

    try
    {
        // We want to use the logging interface that returns the event log
        // id's of the newly created logs (org.open_power.Logging.PEL) so
        // find the service that implements this interface.
        constexpr auto interface = "org.open_power.Logging.PEL";
        constexpr auto path      = "/xyz/openbmc_project/logging";
        std::string service;

        if (0 == util::dbus::findService(interface, path, service))
        {
            // Use function that returns log id's
            constexpr auto function = "CreatePELWithFFDCFiles";

            // Get access to logging interface and method for creating log.
            auto bus = sdbusplus::bus::new_default_system();

            // Using direct create method (for additional data).
            auto method =
                bus.new_method_call(service.c_str(), path, interface, function);

            // The "Create" method requires manually adding the process ID.
            logData["_PID"] = std::to_string(getpid());

            // Get the message registry entry for this failure.
            auto message = __getMessageRegistry(i_servData.getAnalysisType());

            // Get the message severity for this failure.
            auto severity = __getMessageSeverity(i_servData.getAnalysisType());

            // Add the message, with additional log and user data.
            method.append(message, severity, logData, userData);

            // Log the event.
            auto reply = bus.call(method);

            // Response will be a tuple containing bmc-log-id, pel-log-id
            std::tuple<uint32_t, uint32_t> response = {0, 0};

            // Parse reply for response
            reply.read(response);

            o_plid = std::get<1>(response);
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace::err("Exception while creating event log entry");
        std::string exceptionString = std::string(e.what());
        trace::err(exceptionString.c_str());
    }

    // Return the platorm log ID of the error.
    return o_plid;
}

} // namespace analyzer
