#pragma once

#include <endian.h>
#include <string.h>

#include <hei_includes.hpp>

namespace libhei
{

/** @brief A streaming utility to read a Chip Data File buffer. */
class ChipDataStream
{
  public:
    /**
     * @brief Constructor.
     * @param i_buffer     A pointer to the buffer.
     * @param i_bufferSize The buffer size.
     */
    ChipDataStream(void* i_buffer, size_t i_bufferSize) :
        iv_buffer(i_buffer), iv_bufferSize(i_bufferSize)
    {
        HEI_ASSERT(nullptr != i_buffer);
        HEI_ASSERT(0 < i_bufferSize);
    }

    /** @brief Destructor. */
    ~ChipDataStream() = default;

    /** @brief Copy constructor. */
    ChipDataStream(const ChipDataStream&) = delete;

    /** @brief Assignment operator. */
    ChipDataStream& operator=(const ChipDataStream&) = delete;

  private:
    /** Pointer to the first address of the Chip Data File buffer. */
    const void* const iv_buffer;

    /** The Chip Data File buffer size. */
    const size_t iv_bufferSize;

    /** Current byte index within the buffer. */
    size_t iv_currentIndex = 0;

  public:
    /** @brief Output stream operator. */
    template <class D>
    ChipDataStream& operator>>(D& o_right)
    {
        read(&o_right, sizeof(D));
        return *this;
    }

  private:
    /**
     * @brief Copies the given number of data bytes into the buffer from the
     *        current index and then increments the index.
     * @param o_buf  The output buffer.
     * @param i_size The number of bytes to copy.
     */
    void read(void* o_buf, size_t i_size)
    {
        // Check for buffer overflow.
        HEI_ASSERT((iv_currentIndex + i_size) <= iv_bufferSize);

        // Copy the buffer.
        memcpy(o_buf, (char*)iv_buffer + iv_currentIndex, i_size);

        // Increment the curent index for the next piece of data.
        iv_currentIndex += i_size;
    }
};

/** @brief Template specialization for uint16_t. */
template <>
inline ChipDataStream& ChipDataStream::operator>>(uint16_t& o_right)
{
    read(&o_right, sizeof(o_right));
    o_right = be16toh(o_right); // Chip Data is big-endian
    return *this;
}

/** @brief Template specialization for int16_t. */
template <>
inline ChipDataStream& ChipDataStream::operator>>(int16_t& o_right)
{
    read(&o_right, sizeof(o_right));
    o_right = be16toh(o_right); // Chip Data is big-endian
    return *this;
}

/** @brief Template specialization for uint32_t. */
template <>
inline ChipDataStream& ChipDataStream::operator>>(uint32_t& o_right)
{
    read(&o_right, sizeof(o_right));
    o_right = be32toh(o_right); // Chip Data is big-endian
    return *this;
}

/** @brief Template specialization for int32_t. */
template <>
inline ChipDataStream& ChipDataStream::operator>>(int32_t& o_right)
{
    read(&o_right, sizeof(o_right));
    o_right = be32toh(o_right); // Chip Data is big-endian
    return *this;
}

/** @brief Template specialization for uint64_t. */
template <>
inline ChipDataStream& ChipDataStream::operator>>(uint64_t& o_right)
{
    read(&o_right, sizeof(o_right));
    o_right = be64toh(o_right); // Chip Data is big-endian
    return *this;
}

/** @brief Template specialization for int64_t. */
template <>
inline ChipDataStream& ChipDataStream::operator>>(int64_t& o_right)
{
    read(&o_right, sizeof(o_right));
    o_right = be64toh(o_right); // Chip Data is big-endian
    return *this;
}

} // namespace libhei
