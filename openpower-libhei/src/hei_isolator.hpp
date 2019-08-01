#pragma once

#include "hei_includes.hpp"
#include "hei_isolation_data.hpp"
#include "chip_data/hei_chip_data.hpp"

namespace libhei
{

/**
 * @brief This is the main API for Hardware Error Isolation (aka the isolator).
 *
 * The intended flow is to:
 *  - Create a singleton instance of an Isolator object.
 *  - Use initialize() to input each necessary Chip Data File (provided by the
 *    user application).
 *  - Call isolate() each time you want to find all active errors being
 *    reported by a chip.
 *  - Once isolation is no longer needed, the Isolator singleton can be deleted
 *    to free up internal resources.
 *
 * The purpose of the singleton instance is to avoid initializing the object
 * each time isolation is required. The data provided by the Chip Data Files is
 * static. So reinitializing would be a waste of time, unless for some reason
 * the Chip Data Files themselves are updated, which would require
 * reinitialization anyway. Of course, leaving the object in memory chews up
 * resources. So, some users may need to weigh performance vs. memory usage.
 */
class Isolator
{
  public:

    /** @brief Default constructor. */
    Isolator() = default;

    /** @brief Destructor. */
    ~Isolator() = default;

    /** @brief Copy constructor. */
    Isolator( const Isolator & ) = delete;

    /** @brief Assignment operator. */
    Isolator & operator=( const Isolator & ) = delete;

    /**
     * @brief Initializes internal isolation objects based on data from the
     *        given Chip Data File.
     *
     * This function only takes one Chip Data File at a time. Therefore, the
     * user application must call this function for each Chip Data File required
     * for isolation.
     *
     * Storage and management of the Chip Data Files will vary per user
     * application. Therefore, the user application is responsible for loading
     * the Chip Data Files into memory, as needed, and providing the location
     * and size of the data.
     *
     * Once initialization has successfully completed with a Chip Data File, the
     * file is no longer needed in memory.
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
     *                     initialized. This is useful if for some reason the
     *                     Chip Data File is for a chip type has been updated.
     *                     If this function is called and a chip type has
     *                     already been initialized:
     *                      - false (default), the function will return
     *                        RC_CDF_INITIALIZED and exit.
     *                      - true, the function will delete previous isolation
     *                        objects for this chip type and reinitialize.
     *
     * @return RC_SUCCESS or RC_CDF_INVALID or RC_CDF_INITIALIZED
     */
    ReturnCode initialize( void * i_buffer, size_t i_bufferSize,
                           bool i_forceInit = false );

    /**
     * @brief Isolates all active hardware errors found on the given chip.
     *
     * This functions requires initialize() to be called with the Chip Data File
     * corresponding to the given chip type.
     *
     * @param i_chip     This is simply a pointer to a user application object
     *                   that represents the target chip. The isolator does not
     *                   know anything about this object or how to use it. This
     *                   parameter's only purpose is to eventually get passed
     *                   back to the user application in a deviceRead()
     *                   operation.
     *
     * @param i_chipType Each Chip Data File contains a unique chip type
     *                   identifier. The user application will need to input
     *                   this value so the isolator will know which set of the
     *                   isolation objects to reference.
     *
     * @param o_isoData  This return object will contain a list of all active
     *                   hardware errors on this chip, the contents of any
     *                   registers associated with the active errors, and any
     *                   other data that can be useful for debug.
     *
     * @return RC_SUCCESS or RC_CHIP_DATA_MISSING
     */
    ReturnCode isolate( void * i_chip, ChipType_t i_chipType,
                        IsolationData & o_isoData );

  private:

}; // end class Isolator

} // end namespace libhei
