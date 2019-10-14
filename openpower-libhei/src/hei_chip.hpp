#pragma once

#include <hei_types.hpp>

namespace libhei
{

/**
 * @brief This is a simple container for pointers to the user application chip
 *        objects and the associated chip type.
 */
class Chip
{
  public: // Constructors, destructors, assignment, etc.

    Chip() = default;

    ~Chip() = default;

    Chip( const Chip & ) = default;

    Chip & operator=( const Chip & ) = default;

    /**
     * @brief Constructor.
     * @param i_chip See description for iv_chip.
     * @param i_type See description for iv_type.
     */
    Chip( void * i_chip, ChipType_t i_type ) :
        iv_chip(i_chip), iv_type(i_type)
    {}

  public: // Accessors

    void * getChip() const { return iv_chip; }

    ChipType_t getType() const { return iv_type; }

  public: // Operators

    bool operator==( const Chip & r ) const
    {
        return (iv_chip == r.iv_chip) &&
               (iv_type == r.iv_type);
    }

    bool operator<( const Chip & r ) const
    {
        return (  iv_chip <  r.iv_chip     ) ||
               ( (iv_chip == r.iv_chip) &&
                 (iv_type <  r.iv_type)    );
    }

  private:

    /**
     * The user application will provide pointers to user application objects
     * that represent chips targeted for analysis. The isolator does not know
     * anything about these objects nor how to use them. The pointers' only
     * purpose is to eventually get passed back to the user application with
     * information associated with each chip.
     */
    void * iv_chip = nullptr;

    /**
     * When doing analysis on a chip, the isolator will need to know the chip
     * type in order to look up the correct information from the Chip Data
     * Files.
     */
    ChipType_t iv_type = CHIP_TYPE_INVALID;

}; // end class Chip

} // end namespace libhei

