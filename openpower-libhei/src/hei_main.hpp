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
 */

#pragma once

#include <hei_includes.hpp>
#include <hei_isolation_data.hpp>
#include <isolator/hei_isolator.hpp>

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
 * @param i_buffer     A pointer to the buffer containing a single Chip
 *                     Data File.
 *
 * @param i_bufferSize The size (in bytes) of the target Chip Data File.
 *
 * @param i_forceInit  It is possible the user application could call this
 *                     function for a chip type that has already been
 *                     initialized. This is useful if for some reason the Chip
 *                     Data File for a specific chip type has been updated. If
 *                     this function is called and a chip type has already been
 *                     initialized:
 *                      - false (default), the function will return
 *                        RC_CDF_INITIALIZED and exit.
 *                      - true, the function will delete the previous isolation
 *                        objects for this chip type and reinitialize.
 *
 * @return RC_SUCCESS or RC_CHIP_DATA_INVALID or RC_CHIP_DATA_INITIALIZED
 */
inline ReturnCode initialize(void * i_buffer, size_t i_bufferSize,
                             bool i_forceInit = false)
{
    return Isolator::getSingleton().initialize(i_buffer, i_bufferSize,
                                               i_forceInit);
}

/**
 * @brief Deletes all internal isolation objects that were created by
 *        initialize().
 */
inline void uninitialize()
{
    Isolator::getSingleton().uninitialize();
}

/**
 * @brief Isolates all active hardware errors found on the given list of chips.
 *
 * This functions requires initialize() to be called with the Chip Data File
 * corresponding to the given chip types.
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
 *
 * @return RC_SUCCESS or RC_CHIP_DATA_MISSING
 */
inline ReturnCode isolate(const std::vector<Chip> & i_chipList,
                          IsolationData & o_isoData)
{
    return Isolator::getSingleton().isolate(i_chipList, o_isoData);
}

} // end namespace libhei
