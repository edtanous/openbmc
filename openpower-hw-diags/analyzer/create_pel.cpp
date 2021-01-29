#include <unistd.h>

#include <hei_main.hpp>
#include <phosphor-logging/elog.hpp>
#include <sdbusplus/bus.hpp>
#include <util/bin_stream.hpp>
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
    FFDC_SIGNATURES   = 0x01,
    FFDC_CAPTURE_DATA = 0x02,

    // For the callout section, the value of '0xCA' is required per the
    // phosphor-logging openpower-pel extention spec.
    FFDC_CALLOUTS = 0xCA,
};

enum FfdcVersion_t : uint8_t
{
    FFDC_VERSION1 = 0x01,
};

//------------------------------------------------------------------------------

bool __isCheckstop(const libhei::IsolationData& i_isoData)
{
    // Look for any signature with a system checkstop attention.
    auto list = i_isoData.getSignatureList();
    auto itr  = std::find_if(list.begin(), list.end(), [](const auto& s) {
        return libhei::ATTN_TYPE_CHECKSTOP == s.getAttnType();
    });

    return list.end() != itr;
}

//------------------------------------------------------------------------------

void __getSrc(const libhei::Signature& i_signature, uint32_t& o_word6,
              uint32_t& o_word7, uint32_t& o_word8)
{
    // [ 0:15] chip model
    // [16:23] reserved space in chip ID
    // [24:31] chip EC level
    o_word6 = i_signature.getChip().getType();

    // [ 0:15] chip position
    // [16:23] unused
    // [24:31] signature attention type
    auto pos  = util::pdbg::getChipPos(i_signature.getChip());
    auto attn = i_signature.getAttnType();

    o_word7 = (pos & 0xffff) << 16 | (attn & 0xff);

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

std::string __getMessageRegistry(bool i_isCheckstop)
{
    // For now, there are only two choices:
    return i_isCheckstop ? "org.open_power.HwDiags.Error.Checkstop"
                         : "org.open_power.HwDiags.Error.Predictive";
}

//------------------------------------------------------------------------------

std::string __getMessageSeverity(bool i_isCheckstop)
{
    // We could specify the PEL severity in the message registry entry. However,
    // that would require multiple copies of each entry for each possible
    // severity. As a workaround, we will not explicitly state the PEL severity
    // in the message registry. Instead, the message severity will be converted
    // into a PEL severity via the openpower-pels extention of phosphor-logging.

    // Initially, we'll use a severity that will generate a predictive PEL. This
    // is intended for Terminate Immediate (TI) errors and will require service.
    LogSvr::Entry::Level severity = LogSvr::Entry::Level::Warning;

    // If the reason for analysis was due to a system checsktop, the severity
    // will be upgraded to a unrecoverable PEL.
    if (i_isCheckstop)
        severity = LogSvr::Entry::Level::Error;

    // Convert the message severity to a string.
    return LogSvr::Entry::convertLevelToString(severity);
}

//------------------------------------------------------------------------------

void createPel(const libhei::Signature& i_rootCause,
               const libhei::IsolationData& i_isoData)
{
    // The message registry will require additional log data to fill in keywords
    // and additional log data.
    std::map<std::string, std::string> logData;

    // Keep track of the temporary files associated with the user data FFDC.
    // WARNING: Once the objects stored in this vector go out of scope, the
    //          temporary files will be deleted. So they must remain in scope
    //          until the PEL is submitted.
    std::vector<util::FFDCFile> userDataFiles;

    // In several cases, it is important to know if the reason for analysis was
    // due to a system checsktop.
    bool isCheckstop = __isCheckstop(i_isoData);

    // Set words 6-9 of the SRC.
    __setSrc(i_rootCause, logData);

    // Capture the complete signature list.
    __captureSignatureList(i_isoData, userDataFiles);

    // Now, that all of the user data files have been created, transform the
    // data into the proper format for the PEL.
    std::vector<util::FFDCTuple> userData;
    util::transformFFDC(userDataFiles, userData);

    // Get access to logging interface and method for creating log.
    auto bus = sdbusplus::bus::new_default_system();

    // Using direct create method (for additional data).
    auto method = bus.new_method_call(
        "xyz.openbmc_project.Logging", "/xyz/openbmc_project/logging",
        "xyz.openbmc_project.Logging.Create", "CreateWithFFDCFiles");

    // The "Create" method requires manually adding the process ID.
    logData["_PID"] = std::to_string(getpid());

    // Get the message registry entry for this failure.
    auto message = __getMessageRegistry(isCheckstop);

    // Get the message severity for this failure.
    auto severity = __getMessageSeverity(isCheckstop);

    // Add the message, with additional log and user data.
    method.append(message, severity, logData, userData);

    // Log the event.
    bus.call_noreply(method);
}

} // namespace analyzer
