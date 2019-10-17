#pragma once

#include <hei_includes.hpp>
#include <isolator/hei_signature.hpp>

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
  public: // Constructors, destructor, assignment, etc.

    /** @brief Default constructor. */
    IsolationData() = default;

    /** @brief Destructor. */
    ~IsolationData() = default;

    /** @brief Copy constructor. */
    IsolationData( const IsolationData & ) = default;

    /** @brief Assignment operator. */
    IsolationData & operator=( const IsolationData & ) = default;

  private: // Instance variables

    /** A list of all signatures found during isolation. */
    std::vector<Signature> iv_sigLists;

    // TODO: add register dump.

  public: // Member functions

    /**
     * @brief Adds a signature to the signature list.
     * @param i_signature The target signature.
     */
    void addSignature( const Signature & i_signature )
    {
        iv_sigLists.push_back( i_signature );
    }

    /** @brief Allows access to the signature list. */
    const std::vector<Signature> & getSignatureList()
    {
        return iv_sigLists;
    }

    /** @brief Flushes the data to ensure a clean slate for isolation. */
    void flush()
    {
        iv_sigLists.clear();
    }

}; // end class IsolationData

} // end namespace libhei

