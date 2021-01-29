#pragma once

#include <endian.h>
#include <string.h>

#include <filesystem>
#include <fstream>

namespace util
{

/**
 * @brief A streaming utility to read a binary file.
 * @note  IMPORTANT: Assumes file data is in big-endian format.
 */
class BinFileReader
{
  public:
    /**
     * @brief Constructor.
     * @param f The name of the target file.
     */
    explicit BinFileReader(const std::filesystem::path& p) :
        iv_stream(p, std::ios::binary)
    {}

    /** @brief Destructor. */
    ~BinFileReader() = default;

    /** @brief Copy constructor. */
    BinFileReader(const BinFileReader&) = delete;

    /** @brief Assignment operator. */
    BinFileReader& operator=(const BinFileReader&) = delete;

  private:
    /** The input file stream. */
    std::ifstream iv_stream;

  public:
    /** @return True, if the state of the stream is good. */
    bool good()
    {
        return iv_stream.good();
    }

    /**
     * @brief Extracts n characters from the stream and stores them in the array
     *        pointed to by s.
     * @note  This function simply copies a block of data without checking its
     *        contents or endianness.
     * @note  After calling, check good() to determine if the operation was
     *        successful.
     * @param s Pointer to an array of at least n characters.
     * @param n Number of characters to extract.
     */
    void read(void* s, size_t n)
    {
        iv_stream.read(static_cast<char*>(s), n);
    }

    /**
     * @brief Input stream operator.
     * @note  The default template is intentionally not defined so that only
     *        specializations of this function can be used. This avoids
     *        accidental usage on objects where endianness is a concern.
     * @note  This is written as a template so that users can define their own
     *        specializations for non-standard types.
     */
    template <class D>
    BinFileReader& operator>>(D& r);
};

/** @brief Extracts big-endian data to host uint8_t. */
template <>
inline BinFileReader& BinFileReader::operator>>(uint8_t& r)
{
    read(&r, sizeof(r));
    return *this;
}

/** @brief Extracts big-endian data to host uint16_t. */
template <>
inline BinFileReader& BinFileReader::operator>>(uint16_t& r)
{
    read(&r, sizeof(r));
    r = be16toh(r);
    return *this;
}

/** @brief Extracts big-endian data to host uint32_t. */
template <>
inline BinFileReader& BinFileReader::operator>>(uint32_t& r)
{
    read(&r, sizeof(r));
    r = be32toh(r);
    return *this;
}

/** @brief Extracts big-endian data to host uint64_t. */
template <>
inline BinFileReader& BinFileReader::operator>>(uint64_t& r)
{
    read(&r, sizeof(r));
    r = be64toh(r);
    return *this;
}

/**
 * @brief A streaming utility to write a binary file.
 * @note  IMPORTANT: Assumes file data is in big-endian format.
 */
class BinFileWriter
{
  public:
    /**
     * @brief Constructor.
     * @param f The name of the target file.
     */
    explicit BinFileWriter(const std::filesystem::path& p) :
        iv_stream(p, std::ios::binary)
    {}

    /** @brief Destructor. */
    ~BinFileWriter() = default;

    /** @brief Copy constructor. */
    BinFileWriter(const BinFileWriter&) = delete;

    /** @brief Assignment operator. */
    BinFileWriter& operator=(const BinFileWriter&) = delete;

  private:
    /** The output file stream. */
    std::ofstream iv_stream;

  public:
    /** @return True, if the state of the stream is good. */
    bool good()
    {
        return iv_stream.good();
    }

    /**
     * @brief Inserts the first n characters of the the array pointed to by s
              into the stream.
     * @note  This function simply copies a block of data without checking its
     *        contents or endianness.
     * @note  After calling, check good() to determine if the operation was
     *        successful.
     * @param s Pointer to an array of at least n characters.
     * @param n Number of characters to insert.
     */
    void write(void* s, size_t n)
    {
        iv_stream.write(static_cast<char*>(s), n);
    }

    /**
     * @brief Output stream operator.
     * @note  The default template is intentionally not defined so that only
     *        specializations of this function can be used. This avoids
     *        accidental usage on objects where endianness is a concern.
     * @note  This is written as a template so that users can define their own
     *        specializations for non-standard types.
     */
    template <class D>
    BinFileWriter& operator<<(D r);
};

/** @brief Inserts host uint8_t to big-endian data. */
template <>
inline BinFileWriter& BinFileWriter::operator<<(uint8_t r)
{
    write(&r, sizeof(r));
    return *this;
}

/** @brief Inserts host uint16_t to big-endian data. */
template <>
inline BinFileWriter& BinFileWriter::operator<<(uint16_t r)
{
    r = htobe16(r);
    write(&r, sizeof(r));
    return *this;
}

/** @brief Inserts host uint32_t to big-endian data. */
template <>
inline BinFileWriter& BinFileWriter::operator<<(uint32_t r)
{
    r = htobe32(r);
    write(&r, sizeof(r));
    return *this;
}

/** @brief Inserts host uint64_t to big-endian data. */
template <>
inline BinFileWriter& BinFileWriter::operator<<(uint64_t r)
{
    r = htobe64(r);
    write(&r, sizeof(r));
    return *this;
}

} // namespace util
