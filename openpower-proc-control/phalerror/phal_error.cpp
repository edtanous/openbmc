#include "phal_error.hpp"

#include "create_pel.hpp"

#include <libekb.H>
#include <libipl.H>

#include <iomanip>
#include <phosphor-logging/elog.hpp>
#include <sstream>

namespace openpower
{
namespace pel
{
using namespace phosphor::logging;

namespace detail
{

// keys need to be unique so using counter value to generate unique key
static int counter = 0;

// list of debug traces
static std::vector<std::pair<std::string, std::string>> traceLog;

void processLogTraceCallback(void* private_data, const char* fmt, va_list ap)
{
    va_list vap;
    va_copy(vap, ap);
    std::vector<char> log(1 + std::vsnprintf(nullptr, 0, fmt, ap));
    std::vsnprintf(log.data(), log.size(), fmt, vap);
    va_end(vap);
    std::string logstr(log.begin(), log.end());

    // ignore stray characters in log traces coming from ekb
    // example: "\n", which are good for print to screen
    if (logstr.length() < 5)
    {
        return;
    }

    char timeBuf[80];
    time_t t = time(0);
    tm myTm{};
    gmtime_r(&t, &myTm);
    strftime(timeBuf, 80, "%Y-%m-%d %H:%M:%S", &myTm);

    // key values need to be unique for PEL
    // TODO #openbmc/dev/issues/1563
    // If written to Json no need to worry about unique KEY
    std::stringstream str;
    str << std::setfill('0');
    str << "LOG" << std::setw(3) << counter;
    str << " " << timeBuf;
    traceLog.emplace_back(std::make_pair(str.str(), std::move(logstr)));
    counter++;
}

void processBootErrorCallback(bool status)
{
    log<level::INFO>("processBootCallback ", entry("STATUS=%d", status));
    try
    {
        // If failure in hwp execution
        if (!status)
        {
            FFDCData ffdc = libekb_get_ffdc();
            ffdc.insert(ffdc.end(), traceLog.begin(), traceLog.end());
            openpower::pel::createBootErrorPEL(ffdc);
        }
    }
    catch (std::exception& ex)
    {
        reset();
        throw ex;
    }
    reset();
}

void reset()
{
    // reset the trace log and counter
    traceLog.clear();
    counter = 0;
}
} // namespace detail

void addBootErrorCallbacks()
{
    // set log level to info
    ipl_set_loglevel(IPL_INFO);

    // add callback for debug traces
    ipl_set_logfunc(detail::processLogTraceCallback, NULL);

    // add callback for ipl failures
    ipl_set_error_callback_func(detail::processBootErrorCallback)
}
} // namespace pel
} // namespace openpower
