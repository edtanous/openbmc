#pragma once

#include <util/ffdc_file.hpp>

#include <cstddef> // for size_t
#include <map>
#include <string>
#include <vector>

namespace attn
{

constexpr int maxTraceLen = 64; // characters

constexpr auto pathLogging   = "/xyz/openbmc_project/logging";
constexpr auto levelPelError = "xyz.openbmc_project.Logging.Entry.Level.Error";
constexpr auto eventPelTerminate = "xyz.open_power.Attn.Error.Terminate";

/** @brief Logging level types */
enum level
{
    INFO,
    ERROR
};

/** @brief Logging event types */
enum class EventType
{
    Checkstop     = 0,
    Terminate     = 1,
    Vital         = 2,
    HwDiagsFail   = 3,
    AttentionFail = 4,
    PhalSbeChipop = 5
};

/** @brief Maximum length of a single trace event message */
static const size_t trace_msg_max_len = 255;

/** @brief Create trace message template */
template <level L>
void trace(const char* i_message);

/** @brief Commit special attention TI event to log */
void eventTerminate(std::map<std::string, std::string> i_additionalData,
                    char* i_tiInfoData);

/** @brief Commit SBE vital event to log, returns event log Id */
uint32_t eventVital();

/** @brief Commit attention handler failure event to log */
void eventAttentionFail(int i_error);

} // namespace attn
