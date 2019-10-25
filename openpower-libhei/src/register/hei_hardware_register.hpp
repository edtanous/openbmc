#pragma once

#include <hei_includes.hpp>
#include <register/hei_register.hpp>
#include <util/hei_bit_string.hpp>

namespace libhei
{

/**
 * @brief An abstract class containing information (e.g. address, type, length,
 *        etc.) for an actual hardware register.
 *
 * Hardware access:
 *
 *  Actual hardware access is defined by the user application via the user
 *  interface APIs. In order to tell the user application which chip to target,
 *  the user application will give the isolator a list of pointers to its
 *  objects. They will then be passed into the public functions of this class
 *  and eventually given back to the user application when hardware access is
 *  needed.
 *
 * Register cache:
 *
 *  In order to save memory space, each instance of this class does not store
 *  the contents of the target hardware register. Instead, that data is stored
 *  in a register cache, which is a static variable defined in this class. This
 *  allows us to store only what we need. The cache can also be thought of as a
 *  snapshot of the registers at the time of isolation, which can be useful if
 *  the hardware is still running and register values could change.
 *
 *  In order to ensure stale data isn't used from the cache, call
 *  HardwareRegister::flushAll() before beginning isolation on a new attention.
 *  Also, HardwareRegister::flushAll() should be called when the isolator is
 *  uninitialized before the rest of the isolation objects are deleted.
 */
class HardwareRegister : public Register
{
  public:

    /** @brief Pure virtual destructor. */
    virtual ~HardwareRegister() = 0;

  protected:

    /**
     * @brief Constructor from components.
     * @param i_chipType    Type of chip associated with this register.
     * @param i_id          Unique ID for this register.
     * @param i_instance    Instance of this register
     * @param i_accessLevel Hardware access level for this register.
     */
    HardwareRegister(ChipType_t i_chipType, RegisterId_t i_id,
                     RegisterInstance_t i_instance,
                     RegisterAccessLevel_t i_accessLevel) :
        Register(), iv_chipType(i_chipType), iv_id(i_id),
        iv_instance(i_instance), iv_accessLevel(i_accessLevel)
    {}

  private: // Instance variables

    /** The type of chip associated with register. */
    const ChipType_t iv_chipType;

    /** The unique ID for this register. */
    const RegisterId_t iv_id;

    /** A register may have multiple instances. All of which will have the same
     *  ID. This variable is used to distinguish between each instance of the
     *  register. */
    const RegisterInstance_t iv_instance;

    /** The hardware access level of this register (read/write, read-only,
     *  write-only, etc.). */
    const RegisterAccessLevel_t iv_accessLevel;

  public: // Accessor functions

    /** @return The type of chip associated with this register. */
    ChipType_t getChipType() const { return iv_chipType; }

    /* @return The unique ID for this register. */
    RegisterId_t getId() const { return iv_id; }

    /* @return The instance of this register. */
    RegisterInstance_t getInstance() const { return iv_instance; }

    /** @return The hardware access level of this register. */
    RegisterAccessLevel_t getAccessLevel() const { return iv_accessLevel; }

    // NOTE: The following are determined by child classes.

    /** @return This register's type. */
    virtual RegisterType_t getRegisterType() const = 0;

    /** @return The address of this register. */
    virtual RegisterAddress_t getAddress() const = 0;

    /** @return The size (in bytes) of this register. */
    virtual size_t getSize() const = 0;

  public: // Operators

    /** @brief Equals operator. */
    bool operator==(const HardwareRegister & i_r) const
    {
        // Comparing register type, chip type, and address should be sufficient.
        return (getRegisterType() == i_r.getRegisterType()) &&
               (getChipType()     == i_r.getChipType()    ) &&
               (getAddress()      == i_r.getAddress()     );
    }

    /** @brief Less than operator. */
    bool operator<(const HardwareRegister & i_r) const
    {
        // Comparing register type, chip type, and address should be sufficient.
        if (getRegisterType() < i_r.getRegisterType())
        {
            return true;
        }
        else if (getRegisterType() == i_r.getRegisterType())
        {
            if (getChipType() < i_r.getChipType())
            {
                return true;
            }
            else if (getChipType() == i_r.getChipType())
            {
                return (getAddress() < i_r.getAddress());
            }
        }

        return false;
    }

  public:

    /** Function overloaded from parent Register class. */
    const BitString * getBitString(const Chip & i_chip) const;

#if 0
    /**
     * @brief     Updates bit string contents associated with register
     * @param     i_bs               poiner to bit string
     * @return    Nil
     */
    virtual void setBitString(const BitString * i_bs) ;
#endif

    /**
     * @brief  Reads a register from hardware via the user interface APIs.
     * @param  i_chip  The target chip in which this register belongs.
     * @param  i_force When false, this function will only read from hardware if
     *                 an entry for this instance does not already exist in the
     *                 register cache. When true, the entry in the register
     *                 cache is flushed, if it exists. Then this function will
     *                 read from hardware and update the cache.
     * @return See the return code from the registerRead() user interface API.
     */
    ReturnCode read(const Chip & i_chip, bool i_force = false) const;

#ifndef __HEI_READ_ONLY

    /**
     * @brief  Writes the value stored in the register cache to hardware via the
     *         user interface APIs.
     * @param  i_chip  The target chip in which this register belongs.
     * @return See the return code from the registerWrite() user interface API.
     */
    ReturnCode write(const Chip & i_chip) const;

#endif // __HEI_READ_ONLY

  protected:

    /**
     * @brief  Provides access to this register's BitString.
     *
     * WARNING: Allowing public access to this function may be dangerous. For
     *          now it should be left as protected.
     *
     * @param  i_chip  The target chip in which this register belongs.
     * @return A reference to the BitString.
     */
    BitString & accessBitString(const Chip & i_chip);

  private: // Hardware accessor management functions.

    /** @brief Asserts this register belongs on the target accessor chip. */
    void verifyAccessorChip(const Chip & i_chip) const
    {
        HEI_ASSERT(getChipType() == i_chip.getType());
    }

  private: // Register cache class variable

    /**
     * @brief Caches the contents of registers read from hardware.
     *
     * The goal is to create a snapshot of the hardware register contents as
     * close to the reported attention as possible. This snapshot is then used
     * for additional analysis/debug when needed.
     */
    class Cache
    {
      public:

        /** @brief Default constructor. */
        Cache() = default;

        /** @brief Destructor. */
        ~Cache() = default;

        /** @brief Copy constructor. */
        Cache(const Cache &) = delete;

        /** @brief Assignment operator. */
        Cache & operator=(const Cache &) = delete;

        /**
         * @brief  Queries if a specific entry exists in the cache.
         * @param  i_chip  The target chip.
         * @param  i_hwReg The target register.
         * @return True if the entry exists, false otherwise.
         */
        bool query(const Chip & i_chip,
                   const HardwareRegister * i_hwReg) const;

        /**
         * @brief  Returns the data buffer for the given chip and register.
         * @param  i_chip  The target chip.
         * @param  i_hwReg The target register.
         * @return A reference to the BitString containing the register data.
         * @note   If an entry does not exist in the cache, an entry will be
         *         created and the BitString will be initialized to 0.
         */
        BitString & access(const Chip & i_chip,
                           const HardwareRegister * i_hwReg);

        /** @brief Flushes entire contents from cache. */
        void flush();

        /**
         * @brief Removes a single register from the cache.
         * @param i_chip  The target chip.
         * @param i_hwReg The target register.
         */
        void flush(const Chip & i_chip, const HardwareRegister * i_hwReg);

      private:

        /**
         * @brief Stores a BitStringBuffer for each HardwareRegister per Chip.
         *
         * The HardwareRegister keys will just be pointers to the isolation
         * objects created in the main initialize() API. Those should exist
         * until the main uninitialize() API is called. It is important that the
         * cache is flushed at the beginning of the uninitialize() API before
         * the rest of the isolation objects are deleted.
         *
         * The Chip keys are copies of the objects passed to the isolator
         * because the user application is responsible for storage of the
         * objects passed to the isolator. We don't want to chance a Chip was
         * created as a local variable that goes out of scope, or other similar
         * situations.
         */
        std::map<Chip, std::map<const HardwareRegister*, BitString*>> iv_cache;
    };

    /** This allows all HardwareRegister objects access to the cache. */
    static Cache cv_cache;

  public: // Register cache management functions.

    /** @brief Flushes the entire register cache. */
    static void flushAll() { cv_cache.flush(); }

    /**
     * @brief Flushes this register from the cache.
     * @param  i_chip  The target chip in which this register belongs.
     */
    void flush(const Chip & i_chip) const
    {
        cv_cache.flush(i_chip, this);
    }

  private: // Register cache management functions.

    /**
     * @param  i_chip  The target chip in which this register belongs.
     * @return True if an entry for this register exist in this cache.
     */
    bool queryCache(const Chip & i_chip) const
    {
        return cv_cache.query(i_chip, this);
    }

    /**
     * @param  i_chip  The target chip in which this register belongs.
     * @return A reference to this register's BitString in cache.
     */
    BitString & accessCache(const Chip & i_chip) const
    {
        return cv_cache.access(i_chip, this);
    }
};

} // end namespace libhei
