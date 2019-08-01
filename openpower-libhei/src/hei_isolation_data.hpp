#pragma once

#include "hei_includes.hpp"

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

    /** @brief Default constructor. */
    IsolationData() = default;

    /** @brief Copy constructor. */
    IsolationData( const IsolationData & ) = default;

    /** @brief Assignment operator. */
    IsolationData & operator=( const IsolationData & ) = default;

    /** @brief Destructor. */
    ~IsolationData() = default;

  private:

}; // end class IsolationData

} // end namespace libhei

