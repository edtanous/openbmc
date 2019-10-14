#pragma once

/**
 * @brief Models register.It does not contain target.
 *
 * This class stores the hash id and bit length of scom registers It models
 * registers without maintaining target information. Instances of this class
 * are shared across rule chip objects of same type.Once prd object model is
 * built, instances of this register are saved in flyweight.These instances
 * persist as long as prd object model survives.
 */

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
 *  the user application gives the isolator pointers to its chip objects. As
 *  each chip needs to be accessed, the isolator must store the chip in a
 *  static variable defined in this class. The intended use is:
 *
 *   - Call HardwareRegister::setAccessor() with the target chip.
 *   - Perform all necessary hardware accesses to that chip.
 *   - Call HardwareRegister::clearAccessor() to remove the chip access. This
 *     helps ensure we don't try to access the wrong chip.
 *
 * Register cache:
 *
 *  In order to save memory space, each instance of this class does not store
 *  the contents of the target hardware register. Instead, that data is stored
 *  in a register cache, which a static variable defined in this class. This
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
    HardwareRegister( ChipType_t i_chipType, RegisterId_t i_id,
                      RegisterInstance_t i_instance,
                      RegisterAccessLevel_t i_accessLevel ) :
        Register(), iv_chipType( i_chipType ), iv_id( i_id ),
        iv_instance( i_instance ), iv_accessLevel( i_accessLevel )
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

  public:

#if 0
    /**
     * @brief     constructor
     * @param     i_address        address of the register
     * @param     i_bitLength      bit length of register
     * @param     i_targetType     target type associated with register
     */
    HardwareRegister( uint64_t i_address, uint32_t i_bitLength,
                  TARGETING::TYPE i_targetType, AccessLevel i_access ) :
        Register(),
        iv_bitLength( i_bitLength ),
        iv_chipType( i_targetType ),
        iv_scomAddress( i_address ),
        iv_operationType( i_access )
    {}

    /**
     * @brief     constructor .Added this because we save object of this type in
     * @          FlyweightS
     */
    HardwareRegister():
        Register(),
        iv_bitLength( 0 ),
        iv_chipType( TARGETING::TYPE_NA ),
        iv_scomAddress( 0 ),
        iv_operationType( ACCESS_NONE )
    {}
#endif

    /** Function overloaded from parent Register class. */
    virtual const BitString * getBitString() const { return nullptr; }

#if 0
    /**
     * @brief     Updates bit string contents associated with register
     * @param     i_bs               poiner to bit string
     * @return    Nil
     */

    virtual void SetBitString(const BitString * i_bs) ;

    /**
     * @brief     Returns length of the bits string associated with register
     * @return    length of bit string
     */
    uint32_t GetBitLength(void) const { return iv_bitLength ;}
#endif
    size_t getByteSize() const { return 8; } // TODO

    /**
     * @brief  Reads a register from hardware via the user interface APIs.
     * @param  i_force When false, this function will only read from hardware if
     *                 an entry for this instance does not already exist in the
     *                 register cache. When true, the entry in the register
     *                 cache is flushed, if it exists. Then this function will
     *                 read from hardware and update the cache.
     * @return See the return code from the registerRead() user interface API.
     */
    ReturnCode read( bool i_force = false ) const;

    #ifndef __HEI_READ_ONLY

    /**
     * @brief  Writes the value stored in the register cache to hardware via the
     *         user interface APIs.
     * @return See the return code from the registerWrite() user interface API.
     */
    ReturnCode write() const;

    #endif // __HEI_READ_ONLY

#if 0
    /**
     * @brief     Returns the hash id of register
     * @return    returns  hash id of register
     * @pre       None
     * @post      None
     * @note
     */
    virtual uint16_t GetId(void) const { return iv_shortId; };

    /**
     * @brief     Sets the hash id of register
     * @param     i_id     hash id of register
     * @return    Nil
     */
    virtual void SetId(uint16_t i_id) { iv_shortId = i_id; };

   /**
    * @brief    Returns type of Target associated with register.
    * @return   Refer to function description
    */
   TARGETING::TYPE getChipType()const{ return iv_chipType ;} ;
   /**
    * @brief    Returns scom address of register
    * @return   Refer to function description
    */
   uint64_t GetAddress( ) const {return iv_scomAddress ;};
#endif
   /**
    * @brief     compares two ScomRegisterAccess register for equality
    * @param     i_rightRegister   register to be compared against
    * @return    Returns true if registers are equal false otherwise
    */
   bool operator == ( const HardwareRegister & i_rightRegister ) const ;
   /**
    * @brief     defines < operation for ScomRegisterAccess
    * @param     i_rightRegister   register to be compared against
    * @return    Returns false if i_rightRegisters is less and true otherwise
    */
   bool operator < ( const HardwareRegister & i_rightRegister  ) const ;
#if 0
   /**
    * @brief     defines >= operation for ScomRegisterAccess
    * @param     i_rightRegister   register to be compared against
    * @return    Returns true if registers is >= i_rightRegister false
    *            otherwise
    */
   bool operator >= ( const HardwareRegister & i_rightRegister  ) const;

    /** @return The register access level (see enum AccessLevel). */
    virtual AccessLevel getAccessLevel() const { return iv_operationType; }

    /** @brief Sets the register access level (see enum AccessLevel). */
    virtual void setAccessLevel( AccessLevel i_op ) { iv_operationType = i_op; }

  protected: // Functions

    /**
     * @brief     copy constructor
     * @param     i_scomRegister   scomRegister instance to be copied
     */
    HardwareRegister( const Register & i_scomRegister ):
        Register(),
        iv_bitLength(     i_scomRegister.GetBitLength()   ),
        iv_shortId(       i_scomRegister.GetId()          ),
        iv_chipType(      i_scomRegister.getChipType()    ),
        iv_scomAddress(   i_scomRegister.GetAddress()     ),
        iv_operationType( i_scomRegister.getAccessLevel() )
    {}

   /**
    * @brief    Returns reference to bit string associated with register
    * @return   Refer to function description
   */
    virtual BitString & AccessBitString( );

private: // functions

  friend class CaptureData;

  private: // Data

    uint32_t        iv_bitLength;     // bit length of scom
    uint16_t        iv_shortId;       // unique hash id of register
    TARGETING::TYPE iv_chipType;      // type of target associated with register
    uint64_t        iv_scomAddress;   // scom address associated with regiser
    AccessLevel     iv_operationType; // Operation supported (RO, WO, or RW)

#endif

  private: // Hardware accessor class variable

    /** @brief A simple class that stores the chip used to access hardware. */
    class Accessor
    {
      public:

        /**
         * @brief Constructor.
         * @param i_chip The chip used to access hardware.
         */
        explicit Accessor( const Chip & i_chip ) :
            iv_chip( i_chip )
        {}

        /** @brief Destructor. */
        ~Accessor() = default;

        /** @brief Copy constructor. */
        Accessor( const Accessor & ) = delete;

        /** @brief Assignment operator. */
        Accessor & operator=( const Accessor & ) = delete;

        /** @return The chip used to access hardware. */
        const Chip & getChip() const { return iv_chip; }

      private:

        /**
         * A Chip object provided by the user application. The isolator does not
         * know anything about this object nor how to use it. Its only purpose
         * is to get passed back to the user application for hardware access
         * operations.
         */
        const Chip iv_chip;

    }; // end class Accessor

    /**
     * This allows all HardwareRegister objects access to a chip via the user
     * interface APIs. It is intentially defined as a pointer. It can be set to
     * nullptr to signify that access is restricted at this time. This is useful
     * to prevent users from accidentally accessing registers on the wrong chip.
     * It is recommended to use setAccessor() and clearAccessor() to manage this
     * variable.
     */
    static Accessor * cv_accessor;

  public: // Hardware accessor management functions.

    /**
     * @brief Initializes a new hardware accessor.
     * @param i_chip The chip used to access hardware.
     */
    static void setAccessor( const Chip & i_chip )
    {
        clearAccessor();
        cv_accessor = new Accessor( i_chip );
    }

    /** @brief Deletes the current hardware accessor. */
    static void clearAccessor()
    {
        delete cv_accessor;
        cv_accessor = nullptr;
    }

  private: // Hardware accessor management functions.

    /** @return The chip stored in cv_accessor. */
    const Chip & getAccessorChip() const
    {
        HEI_ASSERT( nullptr != cv_accessor );

#if 0
        // Extra sanity check to verify this register belongs on the target
        // accessor chip.
        HEI_ASSERT( getChipType() != cv_accessor->getChip().getChipType() );
#endif

        return cv_accessor->getChip();
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
        Cache( const Cache & ) = delete;

        /** @brief Assignment operator. */
        Cache & operator=( const Cache & ) = delete;

        /**
         * @brief  Queries if a specific entry exists in the cache.
         * @param  i_chip  The target chip.
         * @param  i_hwReg The target register.
         * @return True if the entry exists, false otherwise.
         */
        bool query( const Chip & i_chip,
                    const HardwareRegister * i_hwReg ) const;

        /**
         * @brief  Returns the data buffer for the given chip and register.
         * @param  i_chip  The target chip.
         * @param  i_hwReg The target register.
         * @return A reference to the BitString containing the register data.
         * @note   If an entry does not exist in the cache, an entry will be
         *         created and the BitString will be initialized to 0.
         */
        BitString & access( const Chip & i_chip,
                            const HardwareRegister * i_hwReg );

        /** @brief Flushes entire contents from cache. */
        void flush();

        /**
         * @brief Removes a single register from the cache.
         * @param i_chip  The target chip.
         * @param i_hwReg The target register.
         */
        void flush( const Chip & i_chip, const HardwareRegister * i_hwReg );

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

  private: // Register cache management functions.

    /** @return True if an entry for this register exist in this cache. */
    bool queryCache() const
    {
        return cv_cache.query( getAccessorChip(), this );
    }

    /** @return A reference to this register's BitString in cache. */
    BitString & accessCache() const
    {
        return cv_cache.access( getAccessorChip(), this );
    }
};

} // end namespace libhei

