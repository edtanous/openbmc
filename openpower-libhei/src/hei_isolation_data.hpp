#pragma once

#include <hei_includes.hpp>
#include <chip_data/hei_chip_data.hpp>

namespace libhei
{

/**
 * @brief Contain a list of all active hardware errors on a chip, the contents
 *        of any registers associated with the active errors, and any other data
 *        that can be useful for debug.
 */
class IsolationData
{
  public:

    /**
     * @brief Default constructor.
     * @param i_chip     See iv_chip.
     * @param i_chipType See iv_chipType.
     */
    IsolationData( void * i_chip, ChipType_t i_chipType ) :
        iv_chip(i_chip), iv_chipType(i_chipType)
    {}

    /** @brief Destructor. */
    ~IsolationData() = default;

    /** @brief Copy constructor. */
    IsolationData( const IsolationData & ) = default;

    /** @brief Assignment operator. */
    IsolationData & operator=( const IsolationData & ) = default;

    /** @return The target chip pointer. */
    void * getChip() const { return iv_chip; }

    /** @return The target chip type. */
    ChipType_t getChipType() const { return iv_chipType; }

    /** @brief Flushes the data to ensure a clean slate for next isolation. */
    void clear() {}

  private:

    /**
     * This is simply a pointer to a user application object that represents
     * the target chip. The isolator does not know anything about this object
     * nor how to use it. This parameter's only purpose is to eventually get
     * passed back to the user application in a hardware access operation, which
     * will be defined by the user application.
     */
    void * const iv_chip;

    /**
     * Each Chip Data File contains a unique chip type identifier. The user
     * application will need to input this value so that the isolator will know
     * which set of the isolation objects to reference.
     */
    const ChipType_t iv_chipType;

    // TODO: add error signature list and register dump.

}; // end class IsolationData

} // end namespace libhei

