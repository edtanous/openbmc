#pragma once

#include "pel_common.hpp"
#include "pel_section.hpp"
#include "stream.hpp"

namespace attn
{
namespace pel
{

/**
 * @class PrivateHeader
 *
 * This represents the Private Header section in a PEL.  It is required,
 * and it is always the first section.
 *
 * The Section base class handles the SectionHeader structure that every
 * PEL section has at offset zero.
 *
 * |--------+----------------------+----------+----------+---------------|
 * | length | byte0                | byte1    | byte2    | byte3         |
 * |--------+----------------------+----------+----------+---------------|
 * | 8      | Section Header                                             |
 * |        |                                                            |
 * |--------+------------------------------------------------------------|
 * | 8      | Timestamp - Creation                                       |
 * |        |                                                            |
 * |--------+------------------------------------------------------------|
 * | 8      | Timestamp - Commit                                         |
 * |        |                                                            |
 * |--------+----------------------+----------+----------+---------------|
 * | 4      | Creator ID           | reserved | reserved | section count |
 * |--------+----------------------+----------+----------+---------------|
 * | 4      | OpenBMC Event Log ID                                       |
 * |--------+------------------------------------------------------------|
 * | 8      | Creator Implementation                                     |
 * |        |                                                            |
 * |--------+------------------------------------------------------------|
 * | 4      | Platform Log ID                                            |
 * |--------+------------------------------------------------------------|
 * | 4      | Log Entry ID                                               |
 * |--------+------------------------------------------------------------|
 */
class PrivateHeader : public Section
{
  public:
    PrivateHeader()                     = delete;
    ~PrivateHeader()                    = default;
    PrivateHeader(const PrivateHeader&) = default;
    PrivateHeader& operator=(const PrivateHeader&) = default;
    PrivateHeader(PrivateHeader&&)                 = default;
    PrivateHeader& operator=(PrivateHeader&&) = default;

    /**
     * @brief Constructor
     *
     * Fills in this class's data fields from raw data.
     *
     * @param[in] pel - the PEL data stream
     *
     */
    explicit PrivateHeader(Stream& pel);

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
    static constexpr size_t flattenedSize()
    {
        return Section::flattenedSize() + sizeof(_createTimestamp) +
               sizeof(_commitTimestamp) + sizeof(_creatorID) +
               sizeof(_reservedByte1) + sizeof(_reservedByte2) +
               sizeof(_sectionCount) + sizeof(_obmcLogID) +
               sizeof(_creatorVersion) + sizeof(_plid) + sizeof(_id);
    }

    /**
     * @brief Get the total number of sections in this PEL
     *
     * @return Number of sections
     */
    uint8_t getSectionCount();

    /**
     * @brief Set the total number of sections in this PEL
     *
     * @param[in] sectionCount - Number of sections
     */
    void setSectionCount(uint8_t sectionCount);

  private:
    /**
     * @brief The creation time timestamp
     */
    uint64_t _createTimestamp;

    /**
     * @brief The commit time timestamp
     */
    uint64_t _commitTimestamp;

    /**
     * @brief The creator ID field
     */
    uint8_t _creatorID;

    /**
     * @brief A reserved byte.
     */
    uint8_t _reservedByte1 = 0;

    /**
     * @brief A reserved byte.
     */
    uint8_t _reservedByte2 = 0;

    /**
     * @brief Total number of sections in the PEL
     */
    uint8_t _sectionCount = 3; // private header, user header, primary src = 3

    /**
     * @brief The OpenBMC event log ID that corresponds to this PEL.
     */
    uint32_t _obmcLogID = 0;

    /**
     * @brief The creator subsystem version field
     */
    uint64_t _creatorVersion;
    // CreatorVersion _creatorVersion;

    /**
     * @brief The platform log ID field
     */
    uint32_t _plid;

    /**
     * @brief The log entry ID field
     */
    uint32_t _id = 0;
};

} // namespace pel
} // namespace attn
