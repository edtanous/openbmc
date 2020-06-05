#pragma once

#include <inttypes.h>
#include <stdio.h>

#include <cstdarg>

#ifndef TEST_TRACE
#include <phosphor-logging/log.hpp>
#endif

namespace trace
{

#ifndef TEST_TRACE
constexpr size_t MSG_MAX_LEN = 256;
#endif

/** @brief Information trace (va_list format). */
inline void inf(const char* format, va_list args)
{
#ifdef TEST_TRACE

    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");

#else

    char msg[MSG_MAX_LEN];
    vsnprintf(msg, MSG_MAX_LEN, format, args);
    phosphor::logging::log<phosphor::logging::level::INFO>(msg);

#endif
}

/** @brief Error trace (va_list format). */
inline void err(const char* format, va_list args)
{
#ifdef TEST_TRACE

    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

#else

    char msg[MSG_MAX_LEN];
    vsnprintf(msg, MSG_MAX_LEN, format, args);
    phosphor::logging::log<phosphor::logging::level::ERR>(msg);

#endif
}

/** @brief Information trace (printf format). */
inline void inf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    trace::inf(format, args);
    va_end(args);
}

/** @brief Error trace (printf format). */
inline void err(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    trace::err(format, args);
    va_end(args);
}

} // namespace trace
