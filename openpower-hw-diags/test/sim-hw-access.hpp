#include <assert.h>

#include <util/pdbg.hpp>

#include <map>

namespace sim
{

class ScomAccess
{
  private:
    /** @brief Default constructor. */
    ScomAccess() = default;

    /** @brief Destructor. */
    ~ScomAccess() = default;

    /** @brief Copy constructor. */
    ScomAccess(const ScomAccess&) = delete;

    /** @brief Assignment operator. */
    ScomAccess& operator=(const ScomAccess&) = delete;

  public:
    /** @brief Provides access to a singleton instance of this object. */
    static ScomAccess& getSingleton()
    {
        static ScomAccess theScomAccess;
        return theScomAccess;
    }

  private:
    /** The SCOM values for each chip and address. */
    std::map<pdbg_target*, std::map<uint64_t, uint64_t>> iv_values;

    /** All addresses that will return a SCOM error. */
    std::map<pdbg_target*, std::map<uint64_t, bool>> iv_errors;

  public:
    /**
     * @brief Stores a SCOM register value, which can be accessed later in test.
     * @param i_target The target chip.
     * @param i_addr   A SCOM address on the given chip.
     * @param i_val    The value of the given address.
     */
    void add(pdbg_target* i_target, uint64_t i_addr, uint64_t i_val)
    {
        assert(nullptr != i_target);

        iv_values[i_target][i_addr] = i_val;
    }

    /**
     * @brief This can be used to specify if a specific SCOM address will return
     *        an error when accessed. This is useful for error path testing.
     * @param i_target The target chip.
     * @param i_addr   A SCOM address on the given chip.
     */
    void error(pdbg_target* i_target, uint64_t i_addr)
    {
        assert(nullptr != i_target);

        iv_errors[i_target][i_addr] = true;
    }

    /**
     * @brief Clears all SCOM value/error data.
     */
    void flush()
    {
        iv_values.clear();
        iv_errors.clear();
    }

    /**
     * @brief  Returns the stored SCOM register value.
     * @param  i_target The target chip.
     * @param  i_addr   A SCOM address on the given chip.
     * @param  o_val    The value of the given address. If the target address
     *                  does not exist in iv_values, a value of 0 is returned.
     * @return Will return 1 if the target address exists in iv_errors.
     *         Otherwise, will return 0 for a successful SCOM access.
     */
    int get(pdbg_target* i_target, uint64_t i_addr, uint64_t& o_val)
    {
        assert(nullptr != i_target);

        if (iv_errors[i_target][i_addr])
        {
            return 1;
        }

        o_val = iv_values[i_target][i_addr];

        return 0;
    }
};

class CfamAccess
{
  private:
    /** @brief Default constructor. */
    CfamAccess() = default;

    /** @brief Destructor. */
    ~CfamAccess() = default;

    /** @brief Copy constructor. */
    CfamAccess(const CfamAccess&) = delete;

    /** @brief Assignment operator. */
    CfamAccess& operator=(const CfamAccess&) = delete;

  public:
    /** @brief Provides access to a singleton instance of this object. */
    static CfamAccess& getSingleton()
    {
        static CfamAccess theCfamAccess;
        return theCfamAccess;
    }

  private:
    /** The CFAM values for each chip and address. */
    std::map<pdbg_target*, std::map<uint32_t, uint32_t>> iv_values;

    /** All addresses that will return a CFAM error. */
    std::map<pdbg_target*, std::map<uint32_t, bool>> iv_errors;

  public:
    /**
     * @brief Stores a CFAM register value, which can be accessed later in test.
     * @param i_target The target chip.
     * @param i_addr   A CFAM address on the given chip.
     * @param i_val    The value of the given address.
     */
    void add(pdbg_target* i_target, uint32_t i_addr, uint32_t i_val)
    {
        assert(nullptr != i_target);

        iv_values[i_target][i_addr] = i_val;
    }

    /**
     * @brief This can be used to specify if a specific CFAM address will return
     *        an error when accessed. This is useful for error path testing.
     * @param i_target The target chip.
     * @param i_addr   A CFAM address on the given chip.
     */
    void error(pdbg_target* i_target, uint32_t i_addr)
    {
        assert(nullptr != i_target);

        iv_errors[i_target][i_addr] = true;
    }

    /**
     * @brief Clears all CFAM value/error data.
     */
    void flush()
    {
        iv_values.clear();
        iv_errors.clear();
    }

    /**
     * @brief  Returns the stored CFAM register value.
     * @param  i_target The target chip.
     * @param  i_addr   A CFAM address on the given chip.
     * @param  o_val    The value of the given address. If the target address
     *                  does not exist in iv_values, a value of 0 is returned.
     * @return Will return 1 if the target address exists in iv_errors.
     *         Otherwise, will return 0 for a successful CFAM access.
     */
    int get(pdbg_target* i_target, uint32_t i_addr, uint32_t& o_val)
    {
        assert(nullptr != i_target);

        if (iv_errors[i_target][i_addr])
        {
            return 1;
        }

        o_val = iv_values[i_target][i_addr];

        return 0;
    }
};

} // namespace sim
