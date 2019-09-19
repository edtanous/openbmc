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

#if 0
// Forward References
class CHIP_CLASS;
class MopsRegisterAccess;
class ExtensibleChip;
#endif

/**
 * @brief Stores information (e.g. address, type, length, etc.) for an actual
 *        hardware register.
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
 */
class HardwareRegister : public Register
{
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

    /**
     * @brief     Returns the pointer to bit string
     * @param     i_type               attention type
     * @return    BitString *   pointer to bit string
     */

    virtual const BitString * GetBitString(ATTENTION_TYPE i_type =
                                                  INVALID_ATTENTION_TYPE) const;
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

    /** @return TRUE if entry for this register exist in this cache. */
    bool queryCache() const;

    /**
     * @brief  Reads register contents from cache.
     * @return Reference to bit string buffer maintained in cache.
     */
    BitString & readCache() const;

    /**
     * @brief     Deletes one or all entry in the cache
     * @param     RuleChip pointer associated with register
     * @return    Nil
     */
    void flushCache( ExtensibleChip *i_pChip = nullptr ) const;

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
};

} // end namespace libhei

