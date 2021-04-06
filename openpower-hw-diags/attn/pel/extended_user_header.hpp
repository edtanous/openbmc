#pragma once

#include "pel_common.hpp"
#include "pel_section.hpp"
#include "stream.hpp"

namespace attn
{
namespace pel
{

constexpr uint8_t extendedUserHeaderVersion = 0x01;
constexpr size_t firmwareVersionSize        = 16;

/**
 * @class ExtendedUserHeader
 *
 * This represents the Extended User Header section in a PEL
 *
 * |----------+---------------------------+-------+-------+-------------------|
 * | length   | byte0                     | byte1 | byte2 | byte3             |
 * |----------+---------------------------+-------+-------+-------------------|
 * | 8        | Section Header                                                |
 * |----------+---------------------------------------------------------------|
 * | 20       | Machine Type/Model/Serial                                     |
 * |----------+---------------------------------------------------------------|
 * | 16       | FW Released Version                                           |
 * |----------+---------------------------------------------------------------|
 * | 16       | FW Sub-system driver ver.                                     |
 * |----------+---------------------------+-------+-------+-------------------|
 * | 4        | Reserved                  | rsvd  | rsvd  | Symptom ID length |
 * |----------+---------------------------+-------+-------+-------------------|
 * | 8        | Event Common Reference Time                                   |
 * |----------+---------------------------------------------------------------|
 * | 4        | Reserved                                                      |
 * |----------+---------------------------------------------------------------|
 * | variable | Symptom ID                                                    |
 * |----------+---------------------------------------------------------------|
 *
 */
class ExtendedUserHeader : public Section
{
  public:
    ExtendedUserHeader()                          = delete;
    ~ExtendedUserHeader()                         = default;
    ExtendedUserHeader(const ExtendedUserHeader&) = default;
    ExtendedUserHeader& operator=(const ExtendedUserHeader&) = default;
    ExtendedUserHeader(ExtendedUserHeader&&)                 = default;
    ExtendedUserHeader& operator=(ExtendedUserHeader&&) = default;

    /**
     * @brief Constructor
     *
     * Fills in this class's data fields from raw data.
     *
     * @param[in] pel - the PEL data stream
     */
    explicit ExtendedUserHeader(Stream& pel);

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
     * @brief Returns the size of this section when flattened into a PEL
     *
     * @return size_t - the size of the section
     */
    size_t flattenedSize()
    {
        return Section::flattenedSize() + mtmsSize + _serverFWVersion.size() +
               _subsystemFWVersion.size() + sizeof(_reserved4B) +
               sizeof(_refTime) + sizeof(_reserved1B1) + sizeof(_reserved1B2) +
               sizeof(_reserved1B3) + sizeof(_symptomIdSize) + _symptomIdSize;
    }

    /**
     * @brief Set the symptom id field in extended user header
     *
     * @param[in] symptomId - The symptom ID to set
     */
    void setSymptomId(const std::string& symptomId);

  private:
    /**
     * @brief The structure that holds the machine TM and SN fields.
     */
    uint8_t _mtms[mtmsSize];

    /**
     * @brief The server firmware version
     *
     * NULL terminated.
     *
     * The release version of the full firmware image.
     */
    std::array<uint8_t, firmwareVersionSize> _serverFWVersion;

    /**
     * @brief The subsystem firmware version
     *
     * NULL terminated.
     *
     * On PELs created on the BMC, this will be the BMC code version.
     */
    std::array<uint8_t, firmwareVersionSize> _subsystemFWVersion;

    /**
     * @brief Reserved
     */
    uint32_t _reserved4B = 0;

    /**
     * @brief Event Common Reference Time
     *
     * This is not used by PELs created on the BMC.
     */
    uint64_t _refTime;

    /**
     * @brief Reserved
     */
    uint8_t _reserved1B1 = 0;

    /**
     * @brief Reserved
     */
    uint8_t _reserved1B2 = 0;

    /**
     * @brief Reserved
     */
    uint8_t _reserved1B3 = 0;

    /**
     * @brief The size of the symptom ID field
     */
    uint8_t _symptomIdSize;

    /**
     * @brief The symptom ID field
     *
     * Describes a unique event signature for the log.
     * Required for serviceable events, otherwise optional.
     * When present, must start with the first 8 characters
     * of the ASCII string field from the SRC.
     *
     * NULL terminated.
     */
    std::vector<uint8_t> _symptomId;
};

} // namespace pel
} // namespace attn
