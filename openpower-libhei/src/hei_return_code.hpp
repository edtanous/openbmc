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
    explicit constexpr ReturnCode(uint32_t i_rc = 0) : iv_rc(i_rc) {}

    /** @brief Default copy constructor. */
    ReturnCode(const ReturnCode&) = default;

    /** @brief Default assignment operator. */
    ReturnCode& operator=(const ReturnCode&) = default;

    /** @brief Default destructor. */
    ~ReturnCode() = default;

    /** @brief Integral type conversion function. */
    operator uint32_t() const
    {
        return iv_rc;
    }

    /** @brief Integral type conversion function. */
    operator uint64_t() const
    {
        return iv_rc;
    }

    /** @brief Equals operator. */
    bool operator==(const ReturnCode& rhs) const
    {
        return rhs.iv_rc == iv_rc;
    }

    /** @brief Not equals operator. */
    bool operator!=(const ReturnCode& rhs) const
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
    uint32_t get() const
    {
        return iv_rc;
    }

  private:

    uint32_t iv_rc; ///< return code value
};

/** Function returned successfully. */
static constexpr ReturnCode RC_SUCCESS                 = ReturnCode();

/** The given Chip Data File is malformed or unsupported. */
static constexpr ReturnCode RC_CHIP_DATA_INVALID       = ReturnCode(0x00000001);

/** The given Chip Data File contains a chip type that has already been
 *  initialized. */
static constexpr ReturnCode RC_CHIP_DATA_INITIALIZED   = ReturnCode(0x00000002);

/** The given chip type has not been initialized. */
static constexpr ReturnCode RC_CHIP_DATA_MISSING       = ReturnCode(0x00000003);

/** Generic return code indicating something along the hardware register access
 *  path failed and the returned data is undefined and should not be used. */
static constexpr ReturnCode RC_REG_ACCESS_FAILURE      = ReturnCode(0x00000004);

} // end namespace libhei
