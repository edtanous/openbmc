/** @file hei_bit_string.cpp
 *  @brief BitString and BitStringBuffer class definitions
 */

#include <hei_includes.hpp>
#include <util/hei_bit_string.hpp>

#include <algorithm>

namespace libhei
{

//##############################################################################
//                             BitString class
//##############################################################################

// number of bits in a uint64_t
const uint64_t BitString::UINT64_BIT_LEN = sizeof(uint64_t) * 8;
// number of bits in a uint8_t
const uint64_t BitString::UINT8_BIT_LEN = sizeof(uint8_t) * 8;

//------------------------------------------------------------------------------

uint64_t BitString::getFieldRight(uint64_t i_pos, uint64_t i_len) const
{
    HEI_ASSERT(nullptr != getBufAddr());      // must to have a valid address
    HEI_ASSERT(0 < i_len);                    // must have at least one bit
    HEI_ASSERT(i_len <= UINT64_BIT_LEN);      // i_len length must be valid
    HEI_ASSERT(i_pos + i_len <= getBitLen()); // field must be within range

    // Get the relative address of this byte and the relative starting position
    // within the byte.
    uint64_t relPos  = 0;
    uint8_t* relAddr = getRelativePosition(relPos, i_pos);

    // Get the length of the target bit field within this byte and the length of
    // the bit field for any remaining bits.
    uint64_t bf_len     = i_len;
    uint64_t remain_len = 0;
    if (UINT8_BIT_LEN < relPos + i_len)
    {
        // The target bit field crosses a byte boundary. So truncate the bit
        // length for this byte and update the remaining length.
        bf_len     = UINT8_BIT_LEN - relPos;
        remain_len = i_len - bf_len;
    }

    // Get the target bit field within this byte (right justified).
    uint8_t bf = *relAddr;
    bf <<= relPos;                 // Mask off uneeded bits on the left side.
    bf >>= UINT8_BIT_LEN - bf_len; // Right justify the value.

    // Check for any remaining bits after this target byte.
    if (0 != remain_len)
    {
        // Recursively call this function on the remaining bits and push them
        // into the right side of the return value.
        uint64_t val = static_cast<uint64_t>(bf) << remain_len;
        return val | getFieldRight(i_pos + bf_len, remain_len);
    }

    // Nothing more to do. Simply return this bit field.
    return bf;
}

//------------------------------------------------------------------------------

void BitString::setFieldLeft(uint64_t i_pos, uint64_t i_len, uint64_t i_val)
{
    HEI_ASSERT(nullptr != getBufAddr());      // must to have a valid address
    HEI_ASSERT(0 < i_len);                    // must have at least one bit
    HEI_ASSERT(i_len <= UINT64_BIT_LEN);      // i_len length must be valid
    HEI_ASSERT(i_pos + i_len <= getBitLen()); // field must be within range

    // Get the relative address of this byte and the relative starting position
    // within the byte.
    uint64_t relPos  = 0;
    uint8_t* relAddr = getRelativePosition(relPos, i_pos);

    // Get the length of the target bit field within this byte and the length of
    // the bit field for any remaining bits.
    uint64_t bf_len     = i_len;
    uint64_t remain_len = 0;
    if (UINT8_BIT_LEN < relPos + i_len)
    {
        // The target bit field crosses a byte boundary. So truncate the bit
        // length for this byte and update the remaining length.
        bf_len     = UINT8_BIT_LEN - relPos;
        remain_len = i_len - bf_len;
    }

    // It is possible there are bits in this byte on either side of the target
    // bit field that must be preserved. Get the length of each of those bit
    // fields.
    uint64_t bf_l_len = relPos;
    uint64_t bf_r_len = UINT8_BIT_LEN - (bf_l_len + bf_len);

    // Get the target bit field from the left justified inputed value.
    uint8_t bf = (i_val >> (UINT64_BIT_LEN - bf_len)) << bf_r_len;

    // Get the bit fields on either side of the target bit field.
    uint64_t bf_l_shift = UINT8_BIT_LEN - bf_l_len;
    uint64_t bf_r_shift = UINT8_BIT_LEN - bf_r_len;

    uint8_t bf_l = *relAddr;
    bf_l >>= bf_l_shift;
    bf_l <<= bf_l_shift;

    uint8_t bf_r = *relAddr;
    bf_r <<= bf_r_shift;
    bf_r >>= bf_r_shift;

    // Combine all three parts of the byte and write it out to memory.
    *relAddr = bf_l | bf | bf_r;

    // Check for any remaining bits after this target byte.
    if (0 != remain_len)
    {
        // Recursively call this function on the remaining bits.
        setFieldLeft(i_pos + bf_len, remain_len, i_val << bf_len);
    }
}

//------------------------------------------------------------------------------

void BitString::setPattern(uint64_t i_sPos, uint64_t i_sLen, uint64_t i_pattern,
                           uint64_t i_pLen)
{

    HEI_ASSERT(nullptr != getBufAddr());        // must to have a valid address
    HEI_ASSERT(0 < i_sLen);                     // must have at least one bit
    HEI_ASSERT(i_sPos + i_sLen <= getBitLen()); // field must be within range
    HEI_ASSERT(0 < i_pLen);                     // must have at least one bit
    HEI_ASSERT(i_pLen <= UINT64_BIT_LEN);       // i_pLen length must be valid

    // Get a bit string for the pattern subset (right justified).
    // Note that we cannot use a BitStringBuffer here because this function
    // could be used in the constructor of BitStringBuffer, which could causes
    // an infinite loop.
    uint8_t a[sizeof(i_pattern)] = {};
    BitString bs{sizeof(i_pattern) * 8, a};
    bs.setFieldRight(0, i_pLen, i_pattern);

    // Iterate the range in chunks the size of i_pLen.
    uint64_t endPos = i_sPos + i_sLen;
    for (uint64_t pos = i_sPos; pos < endPos; pos += i_pLen)
    {
        // The true chunk size is either i_pLen or the leftovers at the end.
        uint64_t len = std::min(i_pLen, endPos - pos);

        // Get this chunk's pattern value, truncate (right justified) if needed.
        uint64_t pattern = bs.getFieldRight(0, len);

        // Set the pattern in this string.
        setFieldRight(pos, len, pattern);
    }
}

//------------------------------------------------------------------------------

void BitString::setString(const BitString& i_sStr, uint64_t i_sPos,
                          uint64_t i_sLen, uint64_t i_dPos)
{
    // Ensure the source parameters are valid.
    HEI_ASSERT(nullptr != i_sStr.getBufAddr());
    HEI_ASSERT(0 < i_sLen); // at least one bit to copy
    HEI_ASSERT(i_sPos + i_sLen <= i_sStr.getBitLen());

    // Ensure the destination has at least one bit available to copy.
    HEI_ASSERT(nullptr != getBufAddr());
    HEI_ASSERT(i_dPos < getBitLen());

    // If the source length is greater than the destination length than the
    // extra source bits are ignored.
    uint64_t actLen = std::min(i_sLen, getBitLen() - i_dPos);

    // The bit strings may be in overlapping memory spaces. So we need to copy
    // the data in the correct direction to prevent overlapping.
    uint64_t sRelOffset = 0, dRelOffset = 0;
    uint8_t* sRelAddr = i_sStr.getRelativePosition(sRelOffset, i_sPos);
    uint8_t* dRelAddr = getRelativePosition(dRelOffset, i_dPos);

    // Copy the data.
    if ((dRelAddr == sRelAddr) && (dRelOffset == sRelOffset))
    {
        // Do nothing. The source and destination are the same.
    }
    else if ((dRelAddr < sRelAddr) ||
             ((dRelAddr == sRelAddr) && (dRelOffset < sRelOffset)))
    {
        // Copy the data forward.
        for (uint64_t pos = 0; pos < actLen; pos += UINT64_BIT_LEN)
        {
            uint64_t len = std::min(actLen - pos, UINT64_BIT_LEN);

            uint64_t value = i_sStr.getFieldRight(i_sPos + pos, len);
            setFieldRight(i_dPos + pos, len, value);
        }
    }
    else // Copy the data backwards.
    {
        // Get the first position of the last chunk (byte aligned).
        uint64_t lastPos = ((actLen - 1) / UINT64_BIT_LEN) * UINT64_BIT_LEN;

        // Start with the last chunk and work backwards.
        for (int32_t pos = lastPos; 0 <= pos; pos -= UINT64_BIT_LEN)
        {
            uint64_t len   = std::min(actLen - pos, UINT64_BIT_LEN);
            uint64_t value = i_sStr.getFieldRight(i_sPos + pos, len);
            setFieldRight(i_dPos + pos, len, value);
        }
    }
}

//------------------------------------------------------------------------------

void BitString::maskString(const BitString& i_mask)
{
    // Get the length of the smallest string.
    uint64_t actLen = std::min(getBitLen(), i_mask.getBitLen());

    for (uint64_t pos = 0; pos < actLen; pos += UINT64_BIT_LEN)
    {
        uint64_t len = std::min(actLen - pos, UINT64_BIT_LEN);

        uint64_t dVal = getFieldRight(pos, len);
        uint64_t sVal = i_mask.getFieldRight(pos, len);

        setFieldRight(pos, len, dVal & ~sVal);
    }
}

//------------------------------------------------------------------------------

bool BitString::isEqual(const BitString& i_str) const
{
    if (getBitLen() != i_str.getBitLen())
        return false; // size not equal

    for (uint64_t pos = 0; pos < getBitLen(); pos += UINT64_BIT_LEN)
    {
        uint64_t len = std::min(getBitLen() - pos, UINT64_BIT_LEN);

        if (getFieldRight(pos, len) != i_str.getFieldRight(pos, len))
            return false; // bit strings do not match
    }

    return true; // bit strings match
}

//------------------------------------------------------------------------------

bool BitString::isZero() const
{
    for (uint64_t pos = 0; pos < getBitLen(); pos += UINT64_BIT_LEN)
    {
        uint64_t len = std::min(getBitLen() - pos, UINT64_BIT_LEN);

        if (0 != getFieldRight(pos, len))
            return false; // something is non-zero
    }

    return true; // everything was zero
}

//------------------------------------------------------------------------------

uint64_t BitString::getSetCount(uint64_t i_pos, uint64_t i_len) const
{
    uint64_t endPos = i_pos + i_len;

    HEI_ASSERT(endPos <= getBitLen());

    uint64_t count = 0;

    for (uint64_t i = i_pos; i < endPos; i++)
    {
        if (isBitSet(i))
            count++;
    }

    return count;
}

//------------------------------------------------------------------------------

bool BitString::operator<(const BitString& i_str) const
{
    // The two bit strings must be the same length. Otherwise, the comparison
    // undefined (i.e. compare from the left vs. right).
    HEI_ASSERT(getBitLen() == i_str.getBitLen());

    for (uint64_t pos = 0; pos < getBitLen(); pos += UINT64_BIT_LEN)
    {
        uint64_t len = std::min(getBitLen() - pos, UINT64_BIT_LEN);

        auto l_str = getFieldRight(pos, len);
        auto r_str = i_str.getFieldRight(pos, len);

        if (l_str < r_str)
        {
            return true;
        }
        // The loop can only continue if the values are equal.
        else if (l_str > r_str)
        {
            return false;
        }
    }

    return false;
}

//------------------------------------------------------------------------------

BitStringBuffer BitString::operator~() const
{
    BitStringBuffer bsb(getBitLen());

    for (uint64_t pos = 0; pos < getBitLen(); pos += UINT64_BIT_LEN)
    {
        uint64_t len = std::min(getBitLen() - pos, UINT64_BIT_LEN);

        uint64_t dVal = getFieldRight(pos, len);

        bsb.setFieldRight(pos, len, ~dVal);
    }

    return bsb;
}

//------------------------------------------------------------------------------

BitStringBuffer BitString::operator&(const BitString& i_bs) const
{
    // Get the length of the smallest string.
    uint64_t actLen = std::min(getBitLen(), i_bs.getBitLen());

    BitStringBuffer bsb(actLen);

    for (uint64_t pos = 0; pos < actLen; pos += UINT64_BIT_LEN)
    {
        uint64_t len = std::min(actLen - pos, UINT64_BIT_LEN);

        uint64_t dVal = getFieldRight(pos, len);
        uint64_t sVal = i_bs.getFieldRight(pos, len);

        bsb.setFieldRight(pos, len, dVal & sVal);
    }

    return bsb;
}

//------------------------------------------------------------------------------

BitStringBuffer BitString::operator|(const BitString& i_bs) const
{
    // Get the length of the smallest string.
    uint64_t actLen = std::min(getBitLen(), i_bs.getBitLen());

    BitStringBuffer bsb(actLen);

    for (uint64_t pos = 0; pos < actLen; pos += UINT64_BIT_LEN)
    {
        uint64_t len = std::min(actLen - pos, UINT64_BIT_LEN);

        uint64_t dVal = getFieldRight(pos, len);
        uint64_t sVal = i_bs.getFieldRight(pos, len);

        bsb.setFieldRight(pos, len, dVal | sVal);
    }

    return bsb;
}

//------------------------------------------------------------------------------

BitStringBuffer BitString::operator>>(uint64_t i_shift) const
{
    BitStringBuffer bsb(getBitLen()); // default all zeros

    if (i_shift < getBitLen())
    {
        // bso overlays bsb, containing the shifted offset.
        BitString bso(bsb.getBitLen() - i_shift, bsb.getBufAddr(), i_shift);

        // Copy this into bso.
        bso.setString(*this);
    }

    return bsb;
}

//------------------------------------------------------------------------------

BitStringBuffer BitString::operator<<(uint64_t i_shift) const
{
    BitStringBuffer bsb(getBitLen()); // default all zeros

    if (i_shift < getBitLen())
    {
        // bso overlays *this, containing the shifted offset.
        BitString bso(this->getBitLen() - i_shift, this->getBufAddr(), i_shift);

        // Copy bso into bsb.
        bsb.setString(bso);
    }

    return bsb;
}

//------------------------------------------------------------------------------

uint8_t* BitString::getRelativePosition(uint64_t& o_relPos,
                                        uint64_t i_absPos) const
{
    HEI_ASSERT(nullptr != getBufAddr()); // must to have a valid address
    HEI_ASSERT(i_absPos < getBitLen());  // must be a valid position

    o_relPos = (i_absPos + iv_offset) % UINT8_BIT_LEN;

    return ((uint8_t*)iv_bufAddr + ((i_absPos + iv_offset) / UINT8_BIT_LEN));
}

//##############################################################################
//                          BitStringBuffer class
//##############################################################################

BitStringBuffer::BitStringBuffer(uint64_t i_bitLen) :
    BitString(i_bitLen, nullptr)
{
    initBuffer();
}

//------------------------------------------------------------------------------

BitStringBuffer::~BitStringBuffer()
{
    delete[](uint8_t*) getBufAddr();
}

//------------------------------------------------------------------------------

BitStringBuffer::BitStringBuffer(const BitString& i_bs) :
    BitString(i_bs.getBitLen(), nullptr)
{
    initBuffer();
    if (!i_bs.isZero())
    {
        setString(i_bs);
    }
}

//------------------------------------------------------------------------------

BitStringBuffer::BitStringBuffer(const BitStringBuffer& i_bsb) :
    BitString(i_bsb.getBitLen(), nullptr)
{
    initBuffer();
    if (!i_bsb.isZero())
    {
        setString(i_bsb);
    }
}

//------------------------------------------------------------------------------

BitStringBuffer& BitStringBuffer::operator=(const BitString& i_bs)
{
    // The initBuffer() function will deallocate the buffer as well, however we
    // also need to deallocate the buffer here before we set the length.
    delete[](uint8_t*) getBufAddr();
    setBufAddr(nullptr);

    setBitLen(i_bs.getBitLen());
    initBuffer();
    if (!i_bs.isZero())
    {
        setString(i_bs);
    }

    return *this;
}

//------------------------------------------------------------------------------

BitStringBuffer& BitStringBuffer::operator=(const BitStringBuffer& i_bsb)
{
    if (this != &i_bsb) // Check for assignment to self
    {
        // The initBuffer() function will deallocate the buffer as well, however
        // we also need to deallocate the buffer here before we set the length.
        delete[](uint8_t*) getBufAddr();
        setBufAddr(nullptr);

        setBitLen(i_bsb.getBitLen());
        initBuffer();
        if (!i_bsb.isZero())
        {
            setString(i_bsb);
        }
    }

    return *this;
}

//------------------------------------------------------------------------------

void BitStringBuffer::initBuffer()
{
    // Deallocate the current buffer.
    delete[](uint8_t*) getBufAddr();

    // create new buffer, initialized to 0's
    setBufAddr(new uint8_t[getMinBytes(getBitLen())]());
}

} // end namespace libhei
