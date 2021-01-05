#pragma once

#include "pel_common.hpp"
#include "pel_section.hpp"
#include "stream.hpp"

namespace attn
{
namespace pel
{

/**
 * @class PrimarySrc
 *
 * @brief This class represents the primary SRC sections in the PEL.
 *
 * |--------+--------------------------------------------|
 * | length | field                                      |
 * |--------+--------------------------------------------|
 * | 1      | Version = 0x02                             |
 * |--------+--------------------------------------------|
 * | 1      | Flags = 0x00 (no additional data sections) |
 * |--------+--------------------------------------------|
 * | 1      | reserved                                   |
 * |--------+--------------------------------------------|
 * | 1      | Number of words of hex data + 1 = 0x09     |
 * |--------+--------------------------------------------|
 * | 2      | reserved                                   |
 * |--------+--------------------------------------------|
 * | 2      | Total length of SRC in bytes = 0x48        |
 * |--------+--------------------------------------------|
 * | 4      | Hex Word 2 (word 1 intentionally skipped)  |
 * |--------+--------------------------------------------|
 * | 4      | Hex Word 3                                 |
 * |--------+--------------------------------------------|
 * | 4      | Hex Word 4                                 |
 * |--------+--------------------------------------------|
 * | 4      | Hex Word 5                                 |
 * |--------+--------------------------------------------|
 * | 4      | Hex Word 6                                 |
 * |--------+--------------------------------------------|
 * | 4      | Hex Word 7                                 |
 * |--------+--------------------------------------------|
 * | 4      | Hex Word 8                                 |
 * |--------+--------------------------------------------|
 * | 4      | Hex Word 9                                 |
 * |--------+--------------------------------------------|
 * | 32     | ASCII String                               |
 * |--------+--------------------------------------------|
 */
class PrimarySrc : public Section
{
  public:
    enum HeaderFlags
    {
        additionalSections  = 0x01,
        powerFaultEvent     = 0x02,
        hypDumpInit         = 0x04,
        i5OSServiceEventBit = 0x10,
        virtualProgressSRC  = 0x80
    };

    PrimarySrc()                  = delete;
    ~PrimarySrc()                 = default;
    PrimarySrc(const PrimarySrc&) = delete;
    PrimarySrc& operator=(const PrimarySrc&) = delete;
    PrimarySrc(PrimarySrc&&)                 = delete;
    PrimarySrc& operator=(PrimarySrc&&) = delete;

    /**
     * @brief Constructor
     *
     * Fills in this class's data fields from raw data.
     *
     * @param[in] pel - the PEL data stream
     */
    explicit PrimarySrc(Stream& pel);

    /**
     * @brief Flatten the section into the stream
     *
     * @param[in] stream - The stream to write to
     */
    void flatten(Stream& stream) const override;

    /**
     * @brief Fills in the object from the stream data
     *
     * @param[in] stream - The stream to read from
     */
    void unflatten(Stream& stream);

    /**
     * @brief Set the SRC words
     *
     * @param[in] srcWords - The SRC words
     */
    void setSrcWords(std::array<uint32_t, numSrcWords> srcWords);

    /**
     * @brief Set the ascii string field
     *
     * @param[in]  asciiString - The ascii string
     */
    void setAsciiString(std::array<char, asciiStringSize> asciiString);

  private:
    /**
     * @brief The SRC version field
     */
    uint8_t _version = 0x02;

    /**
     * @brief The SRC flags field
     */
    uint8_t _flags = 0;

    /**
     * @brief A byte of reserved data after the flags field
     */
    uint8_t _reserved1B = 0;

    /**
     * @brief The hex data word count.
     */
    uint8_t _wordCount = numSrcWords + 1; // +1 for backward compatability

    /**
     * @brief Two bytes of reserved data after the hex word count
     */
    uint16_t _reserved2B = 0;

    /**
     * @brief The total size of the SRC section (w/o section header)
     */
    uint16_t _size = 72; // 72 (bytes) = size of basic SRC section

    /**
     * @brief The SRC 'hex words'.
     */
    std::array<uint32_t, numSrcWords> _srcWords;

    /**
     * @brief The 32 byte ASCII character string of the SRC
     */
    std::array<char, asciiStringSize> _asciiString;
};

} // namespace pel
} // namespace attn
