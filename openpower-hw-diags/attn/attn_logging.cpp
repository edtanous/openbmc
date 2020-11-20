#include <unistd.h>

#include <attn/attn_logging.hpp>
#include <phosphor-logging/log.hpp>
namespace attn
{

/** @brief journal entry of type INFO using phosphor logging */
template <>
void trace<INFO>(const char* i_message)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(i_message);
}

/** @brief add an event to the log for PEL generation */
void event(EventType i_event, std::map<std::string, std::string>& i_additional)
{
    bool eventValid = false; // assume no event created

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
            break;
        case EventType::Vital:
            eventName  = "org.open_power.Attn.Error.Vital";
            eventValid = true;
            break;
        case EventType::HwDiagsFail:
            eventName  = "org.open_power.HwDiags.Error.Fail";
            eventValid = true;
            break;
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
        // Get access to logging interface and method for creating log
        auto bus = sdbusplus::bus::new_default_system();

        // using direct create method (for additional data)
        auto method = bus.new_method_call(
            "xyz.openbmc_project.Logging", "/xyz/openbmc_project/logging",
            "xyz.openbmc_project.Logging.Create", "CreateWithFFDCFiles");

        // Create FFDC files containing debug data to store in error log
        std::vector<util::FFDCFile> files{createFFDCFiles()};

        // Create FFDC tuples used to pass FFDC files to D-Bus method
        std::vector<FFDCTuple> ffdcTuples{createFFDCTuples(files)};

        // attach additional data
        method.append(eventName,
                      "xyz.openbmc_project.Logging.Entry.Level.Error",
                      i_additional, ffdcTuples);

        // log the event
        bus.call_noreply(method);
    }
}

/** @brief commit checkstop event to log */
void eventCheckstop(std::map<std::string, std::string>& i_errors)
{
    std::map<std::string, std::string> additionalData;

    // TODO need multi-error/multi-callout stuff here

    // if analyzer isolated errors
    if (!(i_errors.empty()))
    {
        // FIXME TEMP CODE - begin

        std::string signature = i_errors.begin()->first;
        std::string chip      = i_errors.begin()->second;

        additionalData["_PID"]      = std::to_string(getpid());
        additionalData["SIGNATURE"] = signature;
        additionalData["CHIP_ID"]   = chip;

        // FIXME TEMP CODE -end

        event(EventType::Checkstop, additionalData);
    }
}

/** @brief commit special attention TI event to log */
void eventTerminate(std::map<std::string, std::string> i_additionalData)
{
    event(EventType::Terminate, i_additionalData);
}

/** @brief commit SBE vital event to log */
void eventVital()
{
    std::map<std::string, std::string> additionalData;

    additionalData["_PID"] = std::to_string(getpid());

    event(EventType::Vital, additionalData);
}

/** @brief commit analyzer failure event to log */
void eventHwDiagsFail(int i_error)
{
    std::map<std::string, std::string> additionalData;

    additionalData["_PID"] = std::to_string(getpid());

    event(EventType::HwDiagsFail, additionalData);
}

/** @brief commit attention handler failure event to log */
void eventAttentionFail(int i_error)
{
    std::map<std::string, std::string> additionalData;

    additionalData["_PID"]       = std::to_string(getpid());
    additionalData["ERROR_CODE"] = std::to_string(i_error);

    event(EventType::AttentionFail, additionalData);
}

/** @brief parse systemd journal message field */
std::string sdjGetFieldValue(sd_journal* journal, const char* field)
{
    const char* data{nullptr};
    size_t length{0};
    size_t prefix{0};

    // get field value
    if (0 == sd_journal_get_data(journal, field, (const void**)&data, &length))
    {
        // The data returned  by sd_journal_get_data will be prefixed with the
        // field name and "="
        const void* eq = memchr(data, '=', length);
        if (nullptr != eq)
        {
            // get just data following the "="
            prefix = (const char*)eq - data + 1;
        }
        else
        {
            // all the data (should not happen)
            prefix = 0;
            std::string value{}; // empty string
        }

        return std::string{data + prefix, length - prefix};
    }
    else
    {
        return std::string{}; // empty string
    }
}

/** @brief get messages from systemd journal */
std::vector<std::string> sdjGetMessages(const std::string& field,
                                        const std::string& fieldValue,
                                        unsigned int max)
{
    sd_journal* journal;
    std::vector<std::string> messages;

    if (0 == sd_journal_open(&journal, SD_JOURNAL_LOCAL_ONLY))
    {
        SD_JOURNAL_FOREACH_BACKWARDS(journal)
        {
            // Get input field
            std::string value = sdjGetFieldValue(journal, field.c_str());

            // Compare field value and read data
            if (value == fieldValue)
            {
                // Get SYSLOG_IDENTIFIER field (process that logged message)
                std::string syslog =
                    sdjGetFieldValue(journal, "SYSLOG_IDENTIFIER");

                // Get _PID field
                std::string pid = sdjGetFieldValue(journal, "_PID");

                // Get MESSAGE field
                std::string message = sdjGetFieldValue(journal, "MESSAGE");

                // Get timestamp
                uint64_t usec{0};
                if (0 == sd_journal_get_realtime_usec(journal, &usec))
                {

                    // Convert realtime microseconds to date format
                    char dateBuffer[80];
                    std::string date;
                    std::time_t timeInSecs = usec / 1000000;
                    strftime(dateBuffer, sizeof(dateBuffer), "%b %d %H:%M:%S",
                             std::localtime(&timeInSecs));
                    date = dateBuffer;

                    // Store value to messages
                    value = date + " " + syslog + "[" + pid + "]: " + message;
                    messages.insert(messages.begin(), value);
                }
            }

            // limit maximum number of messages
            if (messages.size() >= max)
            {
                break;
            }
        }

        sd_journal_close(journal); // close journal when done
    }

    return messages;
}

/** @brief create a file containing FFDC data */
util::FFDCFile createFFDCFile(const std::vector<std::string>& lines)
{
    // Create FFDC file of type Text
    util::FFDCFile file{util::FFDCFormat::Text};
    int fd = file.getFileDescriptor();

    // Write FFDC lines to file
    std::string buffer;
    for (const std::string& line : lines)
    {
        // Copy line to buffer.  Add newline if necessary.
        buffer = line;
        if (line.empty() || (line.back() != '\n'))
        {
            buffer += '\n';
        }

        // write buffer to file
        write(fd, buffer.c_str(), buffer.size());
    }

    // Seek to beginning of file so error logging system can read data
    lseek(fd, 0, SEEK_SET);

    return file;
}

/** @brief Create FDDC files from journal messages of relevant executables */
std::vector<util::FFDCFile> createFFDCFiles()
{
    std::vector<util::FFDCFile> files{};

    // Executables of interest
    std::vector<std::string> executables{"openpower-hw-diags"};

    for (const std::string& executable : executables)
    {
        try
        {
            // get journal messages
            std::vector<std::string> messages =
                sdjGetMessages("SYSLOG_IDENTIFIER", executable, 30);

            // Create FFDC file containing the journal messages
            if (!messages.empty())
            {
                files.emplace_back(createFFDCFile(messages));
            }
        }
        catch (const std::exception& e)
        {
            std::stringstream ss;
            ss << "createFFDCFiles: " << e.what();
            trace<level::INFO>(ss.str().c_str());
        }
    }

    return files;
}

/** create tuples of FFDC files */
std::vector<FFDCTuple> createFFDCTuples(std::vector<util::FFDCFile>& files)
{
    std::vector<FFDCTuple> ffdcTuples{};
    for (util::FFDCFile& file : files)
    {
        ffdcTuples.emplace_back(
            file.getFormat(), file.getSubType(), file.getVersion(),
            sdbusplus::message::unix_fd(file.getFileDescriptor()));
    }
    return ffdcTuples;
}

} // namespace attn
