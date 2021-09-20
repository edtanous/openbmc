#pragma once

#include <libipl.H>

#include <cstdarg>

namespace openpower
{
namespace pel
{
namespace detail
{

/**
 * @brief Process debug traces
 *
 * Function adds debug traces to the list so that it will be added to the
 * PEL upon failure
 *
 * @param[in] private_data - pointer to private data, unused now
 * @param[in] fmt - format for variable list arguments
 * @param[in] ap - object of va_list, holds information needed to retrieve
 *                 the additional arguments
 */

void processLogTraceCallback(void* private_data, const char* fmt, va_list ap);

/**
 * @brief Process ipl failure/success status
 *
 * If status is success log traces are cleared else used in the
 * creation of failure
 *
 * @param[in] errInfo - Error info structure
 */
void processIplErrorCallback(const ipl_error_info& errInfo);

/**
 * @brief Process boot failure/success status
 *
 * If status is success log traces are cleared else used in the
 * creation of failure
 *
 * @param[in] status - Boot execution status
 */
void processBootError(bool status);

/**
 * @brief Process SBE boot failure/success status
 *
 * This function is used for SBE related failure handling during
 * boot path. For any SBE related boot failure reason code, collects
 * the SBE FFDC using libphal api's. Based on the reason code provided
 * as part of error object collects Dump or PEL.
 * Also resets the trace buffer.
 */
void processSbeBootError();

/**
 * @brief Reset trace log list
 */
void reset();

} // namespace detail

/**
 * @brief Add callbacks for debug traces and boot errors
 *
 * This function adds callback for debug traces and for boot
 * errors
 */
void addBootErrorCallbacks();
} // namespace pel
} // namespace openpower
