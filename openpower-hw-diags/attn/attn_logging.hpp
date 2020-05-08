#pragma once

#include <cstddef> // for size_t
#include <map>
#include <string>

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
void eventTerminate();

/** @brief commit SBE vital event to log */
void eventVital();

/** @brief commit analyzer failure event to log */
void eventHwDiagsFail(int i_error);

/** @brief commit attention handler failure event to log */
void eventAttentionFail(int i_error);

} // namespace attn
