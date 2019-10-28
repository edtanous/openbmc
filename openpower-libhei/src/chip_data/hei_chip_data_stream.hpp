#pragma once

/**
@file hei_chip_data_stream.hpp
@brief Description:  The ChipDataStream class

ChipDataStream makes it possible to read a buffer of binary data using the
stream operators, ">>".

Instantiate ChipDataStream class with a pointer to a data buffer
and its size in bytes as below:

    ChipDataStream cds(streamData,552);

Use the ">>" stream operator then to read the basic integral types
(bool, char, uint8_t, int8_t, uint16_t, int16_t, uint32_t,
int32_t, uint64_t, and int64_t).

Endian issues are taken care of by template specialization.
**/

#include <endian.h>
#include <string.h>

#include <hei_includes.hpp>

namespace libhei
{

/**
 * ChipDataStream
 * ChipDataStream offers convenient stream operator access to binary data.
 **/
class ChipDataStream
{

  private:

    /** iv_buffer points to the first address of the Chip Data File
        buffer.  **/
    const void* const iv_buffer;
    /** iv_bufferSize is the size of the data buffer.  It is
        initialized when the ChipDataStream class is instantiated. **/
    const size_t iv_bufferSize;
    /** iv_asyncOffset keeps an offset into the buffer.  This
        offset is incremented appropriately as data is read. **/
    size_t iv_asyncOffset;

  public:

    /* Constructors */

    /**
    When instantiating the ChipDataStream object a pointer to a
    data buffer containing all the data and its size is passed
    into the class.
    **/
    ChipDataStream(void* i_buffer, size_t i_bufferSize) :
        iv_asyncOffset(0), iv_buffer(i_buffer), iv_bufferSize(i_bufferSize)
    {}

    /** Eliminate copy and assignment operator constructors **/
    ChipDataStream(const ChipDataStream&) = delete;
    ChipDataStream& operator=(const ChipDataStream&) = delete;

    /** Destructor **/

    ~ChipDataStream() = default;

    /** Functions **/

    /**
     *@brief Default case for "operator>>" template.
     *@param             D:       A type
     *@param             o_right: A pointer to where the data is stored
     *@return            *this:   A pointer to "this" object
     **/
    template <class D>
    ChipDataStream& operator>>(D& o_right)
    {
        read(&o_right, sizeof(D));
        return *this;
    }

  private:

    /**
     *@brief Read does the copy from the buffer
     *@param o_buf       a pointer to the location to copy to
     *@param i_size      the size (in bytes) to copy
     *@return            None\n\n
     **/
    void read(void* o_buf, size_t i_size)
    {
        /* Ensure memory is not accessed outside i_buffer */
        HEI_ASSERT((iv_asyncOffset + i_size) <= iv_bufferSize);
        /* Copy appropriate bytes from i_buffer to o_buff */
        memcpy(o_buf, (char*)iv_buffer + iv_asyncOffset, i_size);
        /* Increment asynchronous offset to next piece of data */
        iv_asyncOffset = iv_asyncOffset + i_size;
    }
};

/*
 *   Note:  The specializations below ensure the big-endian Chip Data File
 *          format is converted into the host format.
 */

/**
 *   @brief This template extracts a uint16_t data type from the data buffer
 *   @param             o_right: A pointer to where the data is stored
 *   @return            *this:   A pointer to "this" object
 **/
template <> inline
ChipDataStream& ChipDataStream::operator>>(uint16_t& o_right)
{
    read(&o_right, sizeof(o_right));
    be16toh(o_right);
    return *this;
}

/**@brief This template extracts an int16_t type from the data buffer
 *  @param             o_right: A pointer to where the data is stored
 *  @return            *this:   A pointer to "this" object
 **/
template <> inline
ChipDataStream& ChipDataStream::operator>>(int16_t& o_right)
{
    read(&o_right, sizeof(o_right));
    be16toh(o_right);
    return *this;
}

/**@brief This template extracts a uint32_t type from the data buffer
 *  @param             o_right: A pointer to where the data is stored
 *  @return            *this:   A pointer to "this" object
 **/
template <> inline
ChipDataStream& ChipDataStream::operator>>(uint32_t& o_right)
{
    read(&o_right, sizeof(o_right));
    be32toh(o_right);
    return *this;
}

/**@brief This template extracts an int32_t type from the data buffer
 *  @param             o_right: A pointer to where the data is stored
 *  @return            *this:   A pointer to "this" object
 **/
template <> inline
ChipDataStream& ChipDataStream::operator>>(int32_t& o_right)
{
    read(&o_right, sizeof(o_right));
    be32toh(o_right);
    return *this;
}

/**@brief This template extracts a uint64_t type from the data buffer
 *  @param             o_right: A pointer to where the data is stored
 *  @return            *this:   A pointer to "this" object
 **/
template <> inline
ChipDataStream& ChipDataStream::operator>>(uint64_t& o_right)
{
    read(&o_right, sizeof(o_right));
    be64toh(o_right);
    return *this;
}

/**@brief This template extracts an int64_t type from the data buffer
 *  @param             o_right: A pointer to where the data is stored
 *  @return            *this:   A pointer to "this" object
 **/
template <> inline
ChipDataStream& ChipDataStream::operator>>(int64_t& o_right)
{
    read(&o_right, sizeof(o_right));
    be64toh(o_right);
    return *this;
}

} // namespace libhei
