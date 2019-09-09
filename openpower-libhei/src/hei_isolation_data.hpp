#pragma once

#include <hei_includes.hpp>

namespace libhei
{

/**
 * @brief The main isolate() API is given a list of chips to analyze. This class
 *        will contain a list of all active hardware errors found on those
 *        chips, the contents of any registers associated with the active
 *        errors, and any other data that can be useful for debug.
 */
class IsolationData
{
  public:

    /** @brief Default constructor. */
    IsolationData() = default;

    /** @brief Destructor. */
    ~IsolationData() = default;

    /** @brief Copy constructor. */
    IsolationData( const IsolationData & ) = default;

    /** @brief Assignment operator. */
    IsolationData & operator=( const IsolationData & ) = default;

    /** @brief Flushes the data to ensure a clean slate for next isolation. */
    void clear() {}

  private:

    // TODO: add error signature list and register dump.

}; // end class IsolationData

} // end namespace libhei

