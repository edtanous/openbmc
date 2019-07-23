#pragma once

#include <stdint.h>

namespace libhei
{

class ReturnCode
{
  public:

    /**
     * @brief Constructor.
     * @param i_rc The error code value.
     */
    explicit constexpr ReturnCode( uint32_t i_rc = 0 ) :
        iv_rc( i_rc )
    {}

    /** @brief Default copy constructor. */
    ReturnCode( const ReturnCode & ) = default;

    /** @brief Default assignment operator. */
    ReturnCode& operator=( const ReturnCode & ) = default;

    /** @brief Default destructor. */
    ~ReturnCode() = default;

	/** @brief Integral type conversion function. */
    operator uint32_t() const { return iv_rc; }

    /** @brief Integral type conversion function. */
    operator uint64_t() const { return iv_rc; }

    /** @brief Equals operator. */
    bool operator==( const ReturnCode & rhs ) const
    {
        return rhs.iv_rc == iv_rc;
    }

    /** @brief Not equals operator. */
    bool operator!=( const ReturnCode & rhs ) const
    {
        return rhs.iv_rc != iv_rc;
    }

    /**
     * @brief Returns the error code value.
     *
     * We'd prefer to use the integral type conversion functions above, but
     * some compilers will throw warnings when passing this object into
     * functions with variadic arguments, like printf().
     */
    uint32_t get() const { return iv_rc; }

  private:

    const uint32_t iv_rc; ///< return code value
};

/** Function returned successfully. */
static constexpr ReturnCode RC_SUCCESS = ReturnCode();

}; // end namespace libhei

