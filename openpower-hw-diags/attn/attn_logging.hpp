#pragma once

#include <util/ffdc_file.hpp>

#include <cstddef> // for size_t
#include <map>
#include <string>
#include <vector>

namespace attn
{

/** @brief Logging level types */
enum level
{
    INFO
};

/** @brief Logging event types */
enum class EventType
{
    Checkstop     = 0,
    Terminate     = 1,
    Vital         = 2,
    HwDiagsFail   = 3,
    AttentionFail = 4
};

/** @brief Maximum length of a single trace event message */
static const size_t trace_msg_max_len = 255;

/** @brief create trace message */
template <level L>
void trace(const char* i_message);

/** @brief commit checkstop event to log */
void eventCheckstop(std::map<std::string, std::string>& i_errors);

/** @brief commit special attention TI event to log */
void eventTerminate(std::map<std::string, std::string> i_additionalData);

/** @brief commit SBE vital event to log */
void eventVital();

/** @brief commit analyzer failure event to log */
void eventHwDiagsFail(int i_error);

/** @brief commit attention handler failure event to log */
void eventAttentionFail(int i_error);

using FFDCTuple =
    std::tuple<util::FFDCFormat, uint8_t, uint8_t, sdbusplus::message::unix_fd>;

/**
 * Create an FFDCFile object containing the specified lines of text data.
 *
 * Throws an exception if an error occurs.
 *
 * @param lines lines of text data to write to file
 * @return FFDCFile object
 */
util::FFDCFile createFFDCFile(const std::vector<std::string>& lines);

/**
 * Create FFDCFile objects containing debug data to store in the error log.
 *
 * If an error occurs, the error is written to the journal but an exception
 * is not thrown.
 *
 * @param journal system journal
 * @return vector of FFDCFile objects
 */
std::vector<util::FFDCFile> createFFDCFiles();

/**
 * Create FFDCTuple objects corresponding to the specified FFDC files.
 *
 * The D-Bus method to create an error log requires a vector of tuples to
 * pass in the FFDC file information.
 *
 * @param files FFDC files
 * @return vector of FFDCTuple objects
 */
std::vector<FFDCTuple> createFFDCTuples(std::vector<util::FFDCFile>& files);

} // namespace attn
