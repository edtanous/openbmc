#pragma once

#include <isolator/hei_isolation_chip.hpp>

namespace libhei
{

/**
 * @brief Parses a buffer containing a Chip Data File and builds the necessary
 *        objects.
 *
 * This function will assert:
 *   - The buffer is a properly formated Chip Data File and contains valid data.
 *   - An IsolationChip object has not already been created for the chip type
 *     associated with the given buffer.
 *
 * @param i_buffer     First address of the Chip Data File buffer.
 * @param i_bufferSize Size of the Chip Data File buffer.
 * @param io_isoChips  This map will contain all current IsolationChip objects
 *                     for each of the user applications chip types.
 */
void parseChipDataFile(void* i_buffer, size_t i_bufferSize,
                       IsolationChip::Map& io_isoChips);

} // end namespace libhei
