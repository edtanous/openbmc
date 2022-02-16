#include <unistd.h>

#include <analyzer/analyzer_main.hpp>
#include <attn/attn_common.hpp>
#include <attn/attn_dbus.hpp>
#include <attn/attn_dump.hpp>
#include <attn/attn_logging.hpp>
#include <attn/pel/pel_minimal.hpp>
#include <phosphor-logging/log.hpp>
#include <util/dbus.hpp>
#include <util/ffdc.hpp>
#include <util/trace.hpp>

namespace attn
{

/** @brief Tuple containing information about ffdc files */
using FFDCTuple =
    std::tuple<util::FFDCFormat, uint8_t, uint8_t, sdbusplus::message::unix_fd>;

/**
 * Create FFDCTuple objects corresponding to the specified FFDC files.
 *
 * The D-Bus method to create an error log requires a vector of tuples to
 * pass in the FFDC file information.
 *
 * @param   files - FFDC files
 * @return  vector of FFDCTuple objects
 */
std::vector<FFDCTuple>
    createFFDCTuples(const std::vector<util::FFDCFile>& files)
{
    std::vector<FFDCTuple> ffdcTuples{};
    util::transformFFDC(files, ffdcTuples);

    return ffdcTuples;
}

/**
 * @brief Create an FFDCFile object containing raw data
 *
 * Throws an exception if an error occurs.
 *
 * @param   i_buffer - raw data to add to ffdc faw data file
 * @param   i_size - size of the raw data
 * @return  FFDCFile object
 */
util::FFDCFile createFFDCRawFile(void* i_buffer, size_t i_size)
{
    util::FFDCFile file{util::FFDCFormat::Custom};

    // Write buffer to file and then reset file description file offset
    int fd          = file.getFileDescriptor();
    size_t numBytes = write(fd, static_cast<char*>(i_buffer), i_size);
    if (i_size != numBytes)
    {
        trace::err("%s only %u of %u bytes written", file.getPath().c_str(),
                   numBytes, i_size);
    }

    lseek(fd, 0, SEEK_SET);

    return file;
}

/**
 * Create FFDCFile objects containing debug data to store in the error log.
 *
 * If an error occurs, the error is written to the journal but an exception
 * is not thrown.
 *
 * @param   i_buffer - raw data (if creating raw dump ffdc entry in log)
 * @return  vector of FFDCFile objects
 */
std::vector<util::FFDCFile> createFFDCFiles(char* i_buffer = nullptr,
                                            size_t i_size  = 0)
{
    std::vector<util::FFDCFile> files{};

    // Create raw dump file
    if ((nullptr != i_buffer) && (0 != i_size))
    {
        files.emplace_back(createFFDCRawFile(i_buffer, i_size));
    }

    // Create trace dump file
    util::createFFDCTraceFiles(files);

    return files;
}

/**
 * Create a PEL from an existing PEL
 *
 * Create a new PEL based on the specified raw PEL and submit the new PEL
 * to the backend logging code as a raw PEL. Note that  additional data map
 * here contains data to be committed to the PEL and it can also be used to
 * create the PEL as it contains needed information.
 *
 * @param   i_rawPel - buffer containing a raw PEL
 * @param   i_additional - additional data to be added to the new PEL
 */
void createPelCustom(std::vector<uint8_t>& i_rawPel,
                     std::map<std::string, std::string> i_additional)
{
    // create PEL object from buffer
    auto tiPel = std::make_unique<pel::PelMinimal>(i_rawPel);

    // The additional data contains the TI info as well as the value for the
    // subystem that provided the TI info. Get the subystem from additional
    // data and then populate the prmary SRC and SRC words for the custom PEL
    // based on the sybsystem's TI info.
    uint8_t subsystem = std::stoi(i_additional["Subsystem"]);
    tiPel->setSubsystem(subsystem);

    // If recoverable attentions are active we will call the analyzer and
    // then link the custom pel to analyzer pel.
    std::map<std::string, std::string>::iterator it;
    it = i_additional.find("recoverables");
    if (it != i_additional.end() && "true" == it->second)
    {
        DumpParameters dumpParameters;
        auto plid = analyzer::analyzeHardware(
            analyzer::AnalysisType::TERMINATE_IMMEDIATE, dumpParameters);
        if (0 != plid)
        {
            // Link the PLID if an attention was found and a PEL was generated.
            tiPel->setPlid(plid);
        }
    }

    if (static_cast<uint8_t>(pel::SubsystemID::hypervisor) == subsystem)
    {
        // populate hypervisor SRC words
        tiPel->setSrcWords(std::array<uint32_t, pel::numSrcWords>{
            (uint32_t)std::stoul(i_additional["0x10 SRC Word 12"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x14 SRC Word 13"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x18 SRC Word 14"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x1c SRC Word 15"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x20 SRC Word 16"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x24 SRC Word 17"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x28 SRC Word 18"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x2c SRC Word 19"], 0, 16)});

        // Populate phyp primary SRC

        // char array for raw pel src
        std::array<char, pel::asciiStringSize> srcChars{'0'};

        // src from TI info
        std::string srcString = i_additional["SrcAscii"];

        // copy from string to char array
        srcString.copy(srcChars.data(),
                       std::min(srcString.size(), pel::asciiStringSize), 0);

        tiPel->setAsciiString(srcChars); // pel object src is char array

        // set symptom-id
        auto symptomId = (i_additional["SrcAscii"].substr(0, 8) + '_');

        symptomId += (i_additional["0x10 SRC Word 12"]);
        symptomId += (i_additional["0x14 SRC Word 13"] + '_');
        symptomId += (i_additional["0x18 SRC Word 14"]);
        symptomId += (i_additional["0x1c SRC Word 15"] + '_');
        symptomId += (i_additional["0x20 SRC Word 16"]);
        symptomId += (i_additional["0x24 SRC Word 17"] + '_');
        symptomId += (i_additional["0x28 SRC Word 18"]);
        symptomId += (i_additional["0x2c SRC Word 19"]);

        // setSymptomId will take care of required null-terminate and padding
        tiPel->setSymptomId(symptomId);
    }
    else
    {
        // Populate hostboot SRC words - note HB word 0 from the shared info
        // data (additional data "0x10 HB Word") is reflected in the PEL as
        // "reason code" so we zero it here. Also note that the first word
        // in this group of words starts at word 0 and word 1 does not exits.
        tiPel->setSrcWords(std::array<uint32_t, pel::numSrcWords>{
            (uint32_t)0x00000000,
            (uint32_t)std::stoul(i_additional["0x14 HB Word 2"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x18 HB Word 3"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x1c HB Word 4"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x20 HB Word 5"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x24 HB Word 6"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x28 HB Word 7"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x2c HB Word 8"], 0, 16)});

        // Populate hostboot primary SRC

        // char array for raw pel src
        std::array<char, pel::asciiStringSize> srcChars{'0'};

        // src from TI info
        std::string srcString = i_additional["SrcAscii"];

        // copy from string to char array
        srcString.copy(srcChars.data(),
                       std::min(srcString.size(), pel::asciiStringSize), 0);

        tiPel->setAsciiString(srcChars); // pel object src is char array

        // set symptom-id
        auto symptomId = (i_additional["SrcAscii"].substr(0, 8) + '_');

        symptomId += (i_additional["0x10 HB Word 0"]);       // note: word 1
        symptomId += (i_additional["0x14 HB Word 2"] + '_'); // does not exist
        symptomId += (i_additional["0x18 HB Word 3"]);
        symptomId += (i_additional["0x1c HB Word 4"] + '_');
        symptomId += (i_additional["0x20 HB Word 5"]);
        symptomId += (i_additional["0x24 HB Word 6"] + '_');
        symptomId += (i_additional["0x28 HB Word 7"]);
        symptomId += (i_additional["0x2c HB Word 8"]);

        // setSymptomId will take care of required null-terminate and padding
        tiPel->setSymptomId(symptomId);
    }

    // set severity, event type and action flags
    tiPel->setSeverity(static_cast<uint8_t>(pel::Severity::termination));
    tiPel->setType(static_cast<uint8_t>(pel::EventType::na));
    tiPel->setAction(static_cast<uint16_t>(pel::ActionFlags::service |
                                           pel::ActionFlags::report |
                                           pel::ActionFlags::call));

    // The raw PEL that we used as the basis for this custom PEL contains the
    // attention handler trace data and does not needed to be in this PEL so
    // we remove it here.
    tiPel->setSectionCount(tiPel->getSectionCount() - 1);

    // Update the raw PEL with the new custom PEL data
    tiPel->raw(i_rawPel);

    // create PEL from raw data
    createPelRaw(i_rawPel);
}

/**
 * Log an event handled by the attention handler
 *
 * Basic (non TI) events will generate a standard message-registry based PEL
 *
 * TI events will create two PEL's. One PEL will be informational and will
 * contain trace information relevent to attention handler. The second PEL
 * will be specific to the TI type (including the primary SRC) and will be
 * based off of the TI information provided to the attention handler through
 * shared TI info data area.
 *
 * @param  i_event - The event type
 * @param  i_additional - Additional PEL data
 * @param  i_ffdc - FFDC PEL data
 * @return Event log Id (0 if no event log generated)
 */
uint32_t event(EventType i_event,
               std::map<std::string, std::string>& i_additional,
               const std::vector<util::FFDCFile>& i_ffdc)
{
    uint32_t pelId = 0; // assume no event log generated

    bool eventValid = false; // assume no event created
    bool tiEvent    = false; // assume not a terminate event

    std::string eventName;

    switch (i_event)
    {
        case EventType::Checkstop:
            eventName  = "org.open_power.HwDiags.Error.Checkstop";
            eventValid = true;
            break;
        case EventType::Terminate:
            eventName  = "org.open_power.Attn.Error.Terminate";
            eventValid = true;
            tiEvent    = true;
            break;
        case EventType::Vital:
            eventName  = "org.open_power.Attn.Error.Vital";
            eventValid = true;
            break;
        case EventType::HwDiagsFail:
        case EventType::AttentionFail:
            eventName  = "org.open_power.Attn.Error.Fail";
            eventValid = true;
            break;
        default:
            eventValid = false;
            break;
    }

    if (true == eventValid)
    {
        // Create PEL with additional data and FFDC data. The newly created
        // PEL's platform log-id will be returned.
        pelId = util::dbus::createPel(eventName, levelPelError, i_additional,
                                      createFFDCTuples(i_ffdc));

        // If this is a TI event we will create an additional PEL that is
        // specific to the subsystem that generated the TI.
        if ((0 != pelId) && (true == tiEvent))
        {
            // get file descriptor and size of information PEL
            int pelFd = getPel(pelId);

            // if PEL found, read into buffer
            if (-1 != pelFd)
            {
                auto pelSize = lseek(pelFd, 0, SEEK_END);
                lseek(pelFd, 0, SEEK_SET);

                // read information PEL into buffer
                std::vector<uint8_t> buffer(pelSize);
                size_t numBytes = read(pelFd, buffer.data(), buffer.size());
                if (buffer.size() != numBytes)
                {
                    trace::err("Error reading event log: %u of %u bytes read",
                               numBytes, buffer.size());
                }
                else
                {
                    // create PEL from buffer
                    createPelCustom(buffer, i_additional);
                }

                close(pelFd);
            }

            uint8_t subsystem = std::stoi(i_additional["Subsystem"]);

            // If not hypervisor TI
            if (static_cast<uint8_t>(pel::SubsystemID::hypervisor) != subsystem)
            {
                // Request a dump and transition the host
                if ("true" == i_additional["Dump"])
                {
                    // will not return until dump is complete
                    requestDump(pelId, DumpParameters{0, DumpType::Hostboot});
                }
            }
        }
    }
    return pelId;
}

/**
 * Commit special attention TI event to log
 *
 * Create a event log with provided additional information and standard
 * FFDC data plus TI FFDC data
 *
 * @param i_additional - Additional log data
 * @param i_ti_InfoData - TI FFDC data
 */
void eventTerminate(std::map<std::string, std::string> i_additionalData,
                    char* i_tiInfoData)
{

    uint32_t tiInfoSize = 0; // assume TI info was not available

    if (nullptr != i_tiInfoData)
    {
        tiInfoSize = 56; // assume not hypervisor TI

        uint8_t subsystem = std::stoi(i_additionalData["Subsystem"]);

        // If hypervisor
        if (static_cast<uint8_t>(pel::SubsystemID::hypervisor) == subsystem)
        {
            tiInfoSize = 1024; // assume hypervisor max

            // hypervisor may just want some of the data
            if (0 == (*(i_tiInfoData + 0x09) & 0x01))
            {
                uint32_t* additionalLength = (uint32_t*)(i_tiInfoData + 0x50);
                uint32_t tiAdditional      = be32toh(*additionalLength);
                tiInfoSize = std::min(tiInfoSize, (84 + tiAdditional));
            }
        }
    }

    trace::inf("TI info size = %u", tiInfoSize);

    event(EventType::Terminate, i_additionalData,
          createFFDCFiles(i_tiInfoData, tiInfoSize));
}

/** @brief Commit SBE vital event to log, returns event log ID */
uint32_t eventVital()
{
    // Additional data for log
    std::map<std::string, std::string> additionalData;

    // Create log event with additional data and FFDC data
    return event(EventType::Vital, additionalData, createFFDCFiles(nullptr, 0));
}

/**
 * Commit attention handler failure event to log
 *
 * Create an event log containing the specified error code.
 *
 * @param i_error - Error code
 */
void eventAttentionFail(int i_error)
{
    // Additional data for log
    std::map<std::string, std::string> additionalData;
    additionalData["ERROR_CODE"] = std::to_string(i_error);

    // Create log event with additional data and FFDC data
    event(EventType::AttentionFail, additionalData,
          createFFDCFiles(nullptr, 0));
}

} // namespace attn
