#pragma once

#include <inttypes.h>
#include <stdio.h>

#include <phosphor-logging/log.hpp>

#include <cstdarg>

namespace util
{

/*
 * @brief callback function for logging with log level
 *
 * @param loglevel  PDBG log level
 * @param format    format of output function, same as in printf()
 * @param args      variable list
 *
 * @return none
 */
inline void pdbg_log_callback(const int loglevel, const char* format,
                              va_list args)
{
    constexpr size_t MSG_MAX_LEN = 256;
    char msg[MSG_MAX_LEN];
    vsnprintf(msg, MSG_MAX_LEN, format, args);

    switch (loglevel)
    {
        case PDBG_ERROR:
            phosphor::logging::log<phosphor::logging::level::ERR>(msg);
            break;
        case PDBG_WARNING:
            phosphor::logging::log<phosphor::logging::level::WARNING>(msg);
            break;
        case PDBG_NOTICE:
            phosphor::logging::log<phosphor::logging::level::NOTICE>(msg);
            break;
        case PDBG_INFO:
            phosphor::logging::log<phosphor::logging::level::INFO>(msg);
            break;
        case PDBG_DEBUG:
            phosphor::logging::log<phosphor::logging::level::DEBUG>(msg);
            break;
        default:
            phosphor::logging::log<phosphor::logging::level::ERR>(msg);
            break;
    }
}

} // namespace util
