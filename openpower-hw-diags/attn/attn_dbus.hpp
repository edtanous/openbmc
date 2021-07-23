#pragma once

#include <sdbusplus/bus.hpp>
#include <util/ffdc_file.hpp>

#include <string>

namespace attn
{

/**
 * Create a dbus method
 *
 * Find the dbus service associated with the dbus object path and create
 * a dbus method for calling the specified dbus interface and function.
 *
 * @param i_path - dbus object path
 * @param i_interface - dbus method interface
 * @param i_function - dbus interface function
 * @param o_method - method that is created
 * @param i_extended - optional for extended methods
 * @return non-zero if error
 *
 **/
int dbusMethod(const std::string& i_path, const std::string& i_interface,
               const std::string& i_function,
               sdbusplus::message::message& o_method,
               const std::string& i_extended = "");

/**
 * Create a PEL for the specified event type
 *
 * The additional data provided in the map will be placed in a user data
 * section of the PEL and may additionally contain key words to trigger
 * certain behaviors by the backend logging code. Each set of data described
 * in the vector of ffdc data will be placed in additional user data
 * sections.
 *
 * @param  i_event - the event type
 * @param  i_additional - map of additional data
 * @param  i_ffdc - vector of ffdc data
 * @return Platform log id or 0 if error
 */
uint32_t createPel(const std::string& i_event,
                   std::map<std::string, std::string>& i_additional,
                   const std::vector<util::FFDCTuple>& i_ffdc);

/**
 * Create a PEL from raw PEL data
 *
 * Create a PEL based on the pel defined in the data buffer specified.
 *
 * @param   i_buffer - buffer containing a raw PEL
 */
void createPelRaw(const std::vector<uint8_t>& i_buffer);

/**
 * Get file descriptor of exisitng PEL
 *
 * The backend logging code will search for a PEL having the provided pel id
 * and return a file descriptor of a file containing this pel in raw form.
 *
 * @param  i_pelid - the PEL ID
 * @return file descriptor or -1 if error
 */
int getPel(const uint32_t i_pelId);

} // namespace attn
