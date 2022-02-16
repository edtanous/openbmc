#include <util/ffdc_file.hpp>
#include <util/trace.hpp>

#include <string>
#include <vector>

namespace util
{

/**
 * Parse systemd journal message field
 *
 * Parse the journal looking for the specified field and return the journal
 * data for that field.
 *
 * @param  journal - The journal to parse
 * @param  field - Field containing the data to retrieve
 * @return Data for the speciefied field
 */
std::string sdjGetFieldValue(sd_journal* journal, const char* field)
{
    const char* data{nullptr};
    size_t length{0};

    // get field value
    if (0 == sd_journal_get_data(journal, field, (const void**)&data, &length))
    {
        size_t prefix{0};

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

/**
 * Gather messages from the journal
 *
 * Fetch journal entry data for all entries with the specified field equal to
 * the specified value.
 *
 * @param   field - Field to search on
 * @param   fieldValue -  Value to search for
 * @param   max - Maximum number of messages fetch
 * @return  Vector of journal entry data
 */
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
/**
 * @brief Create an FFDCFile object containing the specified lines of text data
 *
 * Throws an exception if an error occurs.
 *
 * @param   lines - lines of text data to write to file
 * @return  FFDCFile object
 */
FFDCFile createFFDCTraceFile(const std::vector<std::string>& lines)
{
    // Create FFDC file of type Text
    FFDCFile file{FFDCFormat::Text};
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
        size_t numBytes = write(fd, buffer.c_str(), buffer.size());
        if (buffer.size() != numBytes)
        {
            trace::err("%s only %u of %u bytes written", file.getPath().c_str(),
                       numBytes, buffer.size());
        }
    }

    // Seek to beginning of file so error logging system can read data
    lseek(fd, 0, SEEK_SET);

    return file;
}

/**
 * Create FDDC files from journal messages of relevant executables
 *
 * Parse the system journal looking for log entries created by the executables
 * of interest for logging. For each of these entries create a ffdc trace file
 * that will be used to create ffdc log entries. These files will be pushed
 * onto the stack of ffdc files.
 *
 * @param   i_files - vector of ffdc files that will become log entries
 */
void createFFDCTraceFiles(std::vector<FFDCFile>& i_files)
{
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
                i_files.emplace_back(createFFDCTraceFile(messages));
            }
        }
        catch (const std::exception& e)
        {
            trace::inf("createFFDCTraceFiles exception");
            trace::inf(e.what());
        }
    }
}

} // namespace util
