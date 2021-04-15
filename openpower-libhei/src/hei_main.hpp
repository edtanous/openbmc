/**
 * @file hei_main.hpp
 *
 * These are the primary APIs for Hardware Error Isolation (aka the isolator).
 * The intended flow is to:
 *
 *  - Call initialize() for each necessary Chip Data File.
 *
 *  - Call isolate() for all chips that need error isolation.
 *
 *  - Once isolation is no longer needed, call uninitialize() to free up
 *    resources used for isolation.
 *
 * Note that initialize() allocates many objects used for isolation and keeps
 * them in memory. Its purpose is to avoid initializing the objects each time
 * isolation is required. The data provided by the Chip Data Files is static.
 * So reinitializing would be a waste of time, unless for some reason the Chip
 * Data Files themselves are updated, which would require reinitialization
 * anyway. Of course, leaving the object in memory chews up resources. So, some
 * users may need to weigh performance vs. memory usage.
 *
 * If data for a specific chip type changes and needs to be reinitialized, the
 * user application must reinitialize all chip data as follows:
 *
 *  - Call uninitialize() to flush isolation objects for ALL chip types.
 *
 *  - Call initialize() for ALL necessary Chip Data Files.
 */

#pragma once

#include <hei_chip.hpp>
#include <hei_isolation_data.hpp>

#include <vector>

namespace libhei
{

/**
 * @brief Initializes all isolation objects based on data from the given Chip
 *        Data File.
 *
 * This function only takes one Chip Data File at a time. Therefore, the
 * user application must call this function for each Chip Data File required
 * for isolation.
 *
 * Storage and management of the Chip Data Files will vary per user application.
 * Therefore, the user application is responsible for loading the Chip Data
 * Files into memory as needed, and providing the location and size of the data.
 *
 * Once this function returns, the Chip Data File is no longer needed in memory.
 *
 * Details of the Chip Data File format can be found in CHIP_DATA.md.
 *
 * IMPORTANT:
 *   This function will build and store objects as it is parsing the Chip Data
 *   Files. It will assert:
 *     - Each file is the proper format and contains valid data.
 *     - No two files can share the same chip model/level.
 *   This ensures that the isolator does not use a partial (possibly invalid) or
 *   duplicate set of objects.
 *
 * @param i_buffer A pointer to the buffer containing a single Chip Data File.
 *
 * @param i_bufferSize The size (in bytes) of the target Chip Data File.
 */
void initialize(void* i_buffer, size_t i_bufferSize);

/**
 * @brief Deletes all internal isolation objects that were created by
 *        initialize().
 */
void uninitialize();

/**
 * @brief Isolates all active hardware errors found on the given list of chips.
 *
 * This functions will assert that initialize() has been called for each Chip
 * Data File corresponding to the given chip types.
 *
 * @param i_chipList The list of all chips that need to be analyzed. Generally,
 *                   this would include all processor and memory chips in the
 *                   system.
 *
 * @param o_isoData  Initially, all data in the object will be flushed and then
 *                   repopulated with a list of all active hardware errors found
 *                   on the given list of chips, the contents of any registers
 *                   associated with the active errors, and any other data that
 *                   can be useful for debug.
 */
void isolate(const std::vector<Chip>& i_chipList, IsolationData& o_isoData);

/**
 * @brief Return build information string
 *
 * @return Build information string
 */
const char* getBuildInfo();

} // end namespace libhei
