#pragma once

#include <stdint.h>

namespace libhei
{

class BitStringBuffer;

//##############################################################################
//                             BitString class
//##############################################################################

/**
 * A BitString is general purpose class providing the ability to manipulate
 * individual bits within an allocated section of contiguous memory.
 *
 * A BitString does not "own" the memory, it only accesses and manipulates the
 * bits in the range specified. Users will need to ensure memory is allocated
 * and deallocated appropriately. As an alternative, a BitStringBuffer is a
 * BitString that will allocate and maintain its own memory.
 *
 * The length of a BitString is only limited by the amount of memory that
 * contains the data buffer.
 *
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 *  - The bit positions are ordered 0 to n (left to right), where n is the bit
 *    length minus one.
 *  - The data stored in memory is assumed to be in big-endian byte format.
 *
 * So, for example:
 *
 *    uint8_t a[2];                       // 16 bits of memory
 *    BitString bs { 16, a };             // init BitString for a
 *    bs.setFieldRight( 0, 16, 0x1122 );  // set all 16 bits to 0x1122
 *
 * Results in:
 *
 *    a[0] == bs.getFieldRight(0, 8) (i.e. 0x11)
 *    a[1] == bs.getFieldRight(8, 8) (i.e. 0x22)
 *
 * It is very important you do NOT do this:
 *
 *    uint16_t x = 0x1122;      // 16 bits of memory
 *    BitString bs { 16, &x };  // init BitString for x
 *
 * The results are undefined, or at least not portable. For example:
 *
 *   Big-endian:
 *      x is stored in memory as |0x11|0x22|.
 *      Therefore, bs.getFieldRight(0, 8) returns 0x11.
 *
 *   Little-endian:
 *      x is stored in memory as |0x22|0x11|.
 *      Therefore, bs.getFieldRight(0, 8) returns 0x22.
 *
 */
class BitString
{
  private: // constants

    static const uint64_t UINT64_BIT_LEN;
    static const uint64_t UINT8_BIT_LEN;

  public: // functions

    /**
     * @brief Constructor
     * @param i_bitLen  The number of bits in the bit string.
     * @param i_bufAddr The starting address of the memory buffer.
     * @param i_offset  By default, position 0 will be the first bit of the
     *                  buffer's start address. However, this parameter can be
     *                  used to indicate that position 0 actually starts
     *                  somewhere in the middle of the buffer.
     * @pre   Use getMinBytes() to calulate the minimum number of bytes needed
     *        to allocate sufficient memory space for this bit string.
     */
    BitString( uint64_t i_bitLen, void * i_bufAddr,
               uint64_t i_offset = 0 ) :
        iv_bitLen(i_bitLen), iv_bufAddr(i_bufAddr), iv_offset(i_offset)
    {}

    /** @brief Destructor */
    virtual ~BitString() {}

    /** @return The number of bits in the bit string buffer. */
    uint64_t getBitLen() const { return iv_bitLen; }

    /** @return The address of the bit string buffer. Note that this may
     *          return nullptr. */
    void * getBufAddr() const { return iv_bufAddr; }

    /**
     * @param i_bitLen The number of bits for a bit string.
     * @param i_offset Optional starting position of the bit string within the
     *                 memory buffer.
     * @return The minimum number of bytes required to allocate sufficient
     *         memory space for a bit string.
     */
    static uint64_t getMinBytes( uint64_t i_bitLen, uint64_t i_offset = 0 )
    {
        return (i_bitLen + i_offset + UINT8_BIT_LEN-1) / UINT8_BIT_LEN;
    }

    /**
     * @brief  Returns a left-justified value of the given length from the bit
     *         string starting at the given position.
     * @param  i_pos The starting position of the target range.
     * @param  i_len The number of bits of the target range.
     * @return The value of the field range specified (left-justified).
     * @pre    nullptr != getBufAddr()
     * @pre    0 < i_len
     * @pre    i_len <= UINT64_BIT_LEN
     * @pre    i_pos + i_len <= getBitLen()
     */
    uint64_t getFieldLeft( uint64_t i_pos, uint64_t i_len ) const
    {
        return getFieldRight(i_pos, i_len) << (UINT64_BIT_LEN - i_len);
    }

    /**
     * @brief  Returns a right-justified value of the given length from the bit
     *         string starting at the given position.
     * @param  i_pos The starting position of the target range.
     * @param  i_len The number of bits of the target range.
     * @return The value of the field range specified (right-justified).
     * @pre    nullptr != getBufAddr()
     * @pre    0 < i_len
     * @pre    i_len <= UINT64_BIT_LEN
     * @pre    i_pos + i_len <= getBitLen()
     */
    uint64_t getFieldRight( uint64_t i_pos, uint64_t i_len ) const;

    /**
     * @brief  Sets a left-justified value of the given length into the bit
     *         string starting at the given position.
     * @param i_pos The starting position of the target range.
     * @param i_len The number of bits of the target range.
     * @param i_val The left-justified value to set.
     * @pre   nullptr != getBufAddr()
     * @pre   0 < i_len
     * @pre   i_len <= UINT64_BIT_LEN
     * @pre   i_pos + i_len <= getBitLen()
     */
    void setFieldLeft( uint64_t i_pos, uint64_t i_len, uint64_t i_val );

    /**
     * @brief  Sets a right-justified value of the given length into the bit
     *         string starting at the given position.
     * @param i_pos The starting position of the target range.
     * @param i_len The number of bits of the target range.
     * @param i_val The right-justified value to set.
     * @pre   nullptr != getBufAddr()
     * @pre   0 < i_len
     * @pre   i_len <= UINT64_BIT_LEN
     * @pre   i_pos + i_len <= getBitLen()
     */
    void setFieldRight( uint64_t i_pos, uint64_t i_len, uint64_t i_val )
    {
        setFieldLeft( i_pos, i_len, i_val << (UINT64_BIT_LEN - i_len) );
    }

    /**
     * @param  i_pos The target position.
     * @return True if the bit at the given position is set(1), false otherwise.
     * @pre    i_pos < getBitLen().
     */
    bool isBitSet( uint64_t i_pos ) const
    {
        return 0 != getFieldRight(i_pos, 1);
    }

    /**
     * @brief Sets the target position to 1.
     * @param i_pos The target position.
     * @pre   i_pos < getBitLen().
     */
    void setBit( uint64_t i_pos ) { setFieldRight( i_pos, 1, 1 ); }

    /** @brief Sets the entire bit string to 1's. */
    void setAll() { setPattern(UINT64_MAX); }

    /**
     * @brief Sets the target position to 0.
     * @param i_pos The target position.
     * @pre   i_pos < getBitLen().
     */
    void clearBit( uint64_t i_pos ) { setFieldRight( i_pos, 1, 0 ); }

    /** @brief Sets the entire bit string to 0's. */
    void clearAll() { setPattern(0); }

    /**
     * @brief Sets a range within the string based on the pattern and length
     *        provided.
     * @param i_sPos    Starting position of this string.
     * @param i_sLen    The length of the target range.
     * @param i_pattern The pattern to set (right justified).
     * @param i_pLen    The length of the pattern.
     * @pre   nullptr != getBufAddr()
     * @pre   0 < i_sLen
     * @pre   i_sPos + i_sLen <= getBitLen()
     * @pre   0 < i_pLen <= UINT64_BIT_LEN
     * @post  The pattern is repeated/truncated as needed.
     *
     * Examples:  i_sPos(0), i_sLen(10), i_pattern(0xA), i_pLen(4)
     *            Old String: 0000000000
     *            New String: 1010101010
     *
     *            i_sPos(3), i_sLen(4), i_pattern(0x3), i_pLen(3)
     *            Old String: 0001001000
     *            New String: 0000110000
     */
    void setPattern( uint64_t i_sPos, uint64_t i_sLen,
                     uint64_t i_pattern, uint64_t i_pLen );

    /**
     * @brief Sets entire string based on the pattern and length provided.
     * @param i_pattern The pattern to set (right justified).
     * @param i_pLen    The length of the pattern.
     * @note  See definition above for prerequisites.
     * @post  The entire string is filled with the pattern.
     * @post  The pattern is repeated/truncated as needed.
     */
    void setPattern( uint64_t i_pattern, uint64_t i_pLen )
    {
        setPattern( 0, getBitLen(), i_pattern, i_pLen );
    }

    /**
     * @brief Sets entire string based on the pattern provided.
     * @param i_pattern The pattern to set (right justified).
     * @note  See definition above for prerequisites.
     * @post  The entire string is filled with the pattern.
     * @post  The pattern is repeated/truncated as needed.
     */
    void setPattern( uint64_t i_pattern )
    {
        setPattern( i_pattern, sizeof(i_pattern) * 8 );
    }

    /**
     * @brief Set bits in this string based on the given string.
     * @param i_sStr The source string.
     * @param i_sPos The starting position of the source string.
     * @param i_sLen The number of bits to copy from the source string.
     * @param i_dPos The starting position of the this string.
     * @pre   nullptr != getBufAddr()
     * @pre   nullptr != i_sStr.getBufAddr()
     * @pre   0 < i_sLen
     * @pre   i_sPos + i_sLen <= i_sStr.getBitLen()
     * @pre   i_dPos < getBitLen()
     * @post  Source bits in given range are copied to this starting at i_dPos.
     * @note  If the length of the given string is greater than the length of
     *        this string, then the extra bits are ignored.
     * @note  If the length of the given string is less than the length of this
     *        string, then the extra bits in this string are not modified.
     * @note  This string and the source string may specify overlapping memory.
     */
    void setString( const BitString & i_sStr, uint64_t i_sPos,
                    uint64_t i_sLen, uint64_t i_dPos = 0 );

    /**
     * @brief Set bits in this string based on the provided string.
     * @param i_sStr The source string.
     * @note  This will try to copy as much of the source as possible to this
     *        string, starting with the first bit in each string.
     * @note  See the other definition of this function for details and
     *        restrictions.
     */
    void setString( const BitString & i_sStr )
    {
        setString( i_sStr, 0, i_sStr.getBitLen() );
    }

    /**
     * @brief Masks (clears) any bits set in this string that correspond to bits
     *        set in the given string (this & ~mask).
     * @param i_mask The mask string (right justified).
     * @note  If the length of the given string is greater than the length of
     *        this string, then the extra bits are ignored.
     * @note  If the length of the given string is less than the length of this
     *        string, then the extra bits in this string are not modified.
     */
    void maskString( const BitString & i_mask );

    /**
     * @param  i_str The string to compare.
     * @return True if the strings are equivalent, false otherwise.
     * @pre    Both strings must be of equal length and have same values to be
     *         equal.
     */
    bool isEqual( const BitString & i_str ) const;

    /** @return True if there are no bit set(1) in this bit string, false
     *          otherwise. */
    bool isZero() const;

    /**
     * @param  i_pos The starting position of the target range.
     * @param  i_len The length of the target range.
     * @return The number of bits that are set(1) in given range of this string.
     * @pre    nullptr != getBufAddr()
     * @pre    i_pos + i_len <= getBitLen()
     */
    uint64_t getSetCount( uint64_t i_pos, uint64_t i_len ) const;

    /** @return The number of bits that are set(1) in this string. */
    uint64_t getSetCount() const { return getSetCount( 0, getBitLen() ); }

    /** @brief Comparison operator. */
    bool operator==( const BitString & i_str ) const { return isEqual(i_str); }

    /** @brief Bitwise NOT operator. */
    BitStringBuffer operator~() const;

    /** @brief Bitwise AND operator. */
    BitStringBuffer operator&( const BitString & i_bs ) const;

    /** @brief Bitwise OR operator. */
    BitStringBuffer operator|( const BitString & i_bs ) const;

    /** @brief Right shift operator. */
    BitStringBuffer operator>>( uint64_t i_shift ) const;

    /** @brief Left shift operator. */
    BitStringBuffer operator<<( uint64_t i_shift ) const;

    /**
     * @brief Explicitly disables copy from BitString.
     *
     * Prevents assigning a BitString & to a BitString, which would strip
     * polymorphism.
     */
    BitString( const BitString & i_bs ) = delete;

    /**
     * @brief Explicitly disables assignment from BitStringBuffer.
     *
     * Allowing this would be dangerous if the BitStringBuffer goes out of scope
     * because the BitString would point to memory that is no longer in context.
     */
    BitString & operator=( const BitStringBuffer & i_bsb ) = delete;

    /**
     * @brief Explicitly disables copy from BitStringBuffer.
     *
     * Allowing this would be dangerous if the BitStringBuffer goes out of scope
     * because the BitString would point to memory that is no longer in context.
     */
    BitString( const BitStringBuffer & i_bsb ) = delete;

  protected: // functions

    /**
     * @param i_newBufAddr The starting address of the new bit string buffer.
     * @pre   Before calling this function, make sure you deallocate the old
     *        buffer to avoid memory leaks.
     */
    void setBufAddr( void * i_newBufAddr ) { iv_bufAddr = i_newBufAddr; }

    /** @param i_newBitLen The new bit length of this bit string buffer. */
    void setBitLen( uint64_t i_newBitLen ) { iv_bitLen = i_newBitLen; }

  private: // functions

    /**
     * @brief  Given a bit position within the bit string, this function returns
     *         the address that contains the bit position and the bit position
     *         relative to that address.
     * @param  o_relPos The returned relative position.
     * @param  i_absPos The inputted absolute position.
     * @return The relative address.
     * @pre    nullptr != getBufAddr()
     * @pre    i_absPos < getBitLen()
     */
    uint8_t * getRelativePosition( uint64_t & o_relPos,
                                   uint64_t   i_absPos ) const;

  private: // instance variables

    uint64_t   iv_bitLen;  ///< The bit length of this buffer.
    void     * iv_bufAddr; ///< The beginning address of this buffer.
    uint64_t   iv_offset;  ///< Start position offset
};

//##############################################################################
//                          BitStringBuffer class
//##############################################################################

/** A BitStringBuffer is a BitString that maintains its own buffer in memory. It
 *  guarantees that sufficient memory is allocated and deallocated in the
 *  constructor and destructor, respectively. In addition, the assignment
 *  operator will adjust the amount of memory needed, as necessary, for the
 *  assignment. */
class BitStringBuffer : public BitString
{
  public: // functions

    /**
     * @brief Constructor
     * @param i_bitLen Number of bits in the string.
     */
    explicit BitStringBuffer( uint64_t i_bitLen );

    /** @brief Destructor */
    ~BitStringBuffer();

    /** @brief Copy constructor from BitString */
    BitStringBuffer( const BitString & i_bs );

    /** @brief Copy constructor from BitStringBuffer */
    BitStringBuffer( const BitStringBuffer & i_bsb );

    /** @brief Assignment from BitString */
    BitStringBuffer & operator=( const BitString & i_bs );

    /** @brief Assignment from BitStringBuffer */
    BitStringBuffer & operator=( const BitStringBuffer & i_bsb );

  private: // functions

    /** @brief Deallocates the old buffer, if needed, and initializes the new
     *         buffer. */
    void initBuffer();
};

} // end namespace libhei

