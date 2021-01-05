#pragma once

#include "pel_common.hpp"
#include "pel_section.hpp"
#include "stream.hpp"

namespace attn
{
namespace pel
{

/**
 * @class UserHeader
 *
 * This represents the User Header section in a PEL.
 *
 * |--------+----------------+----------------+----------------+------------|
 * | length | byte0          | byte1          | byte2          | byte3      |
 * |--------+----------------+----------------+----------------+------------|
 * | 8      | Section Header                                                |
 * |        |                                                               |
 * |--------+----------------+----------------+----------------+------------|
 * | 4      | Subsystem ID   | Event Scope    | Event Severity | Event Type |
 * |--------+----------------+----------------+----------------+------------|
 * | 4      | reserved                                                      |
 * |--------+----------------+----------------+-----------------------------|
 * | 4      | Problem Domain | Problem Vector | Event Action Flags          |
 * |--------+----------------+----------------+-----------------------------|
 * | 4      | reserved                                                      |
 * |--------+---------------------------------------------------------------|
 *
 */
class UserHeader : public Section
{
  public:
    UserHeader()                  = delete;
    ~UserHeader()                 = default;
    UserHeader(const UserHeader&) = default;
    UserHeader& operator=(const UserHeader&) = default;
    UserHeader(UserHeader&&)                 = default;
    UserHeader& operator=(UserHeader&&) = default;

    /**
     * @brief Constructor
     *
     * Fills in this class's data fields from raw data.
     *
     * @param[in] pel - the PEL data stream
     */
    explicit UserHeader(Stream& pel);

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
        return Section::flattenedSize() + sizeof(_eventSubsystem) +
               sizeof(_eventScope) + sizeof(_eventSeverity) +
               sizeof(_eventType) + sizeof(_reserved4Byte1) +
               sizeof(_problemDomain) + sizeof(_problemVector) +
               sizeof(_actionFlags) + sizeof(_reserved4Byte2);
    }

    /**
     * @brief Set the subsystem field
     *
     * @param[in] subsystem - The subsystem value
     */
    void setSubsystem(uint8_t subsystem);

    /**
     * @brief Set the severity field
     *
     * @param[in] severity - The severity to set
     */
    void setSeverity(uint8_t severity);

    /**
     * @brief Set the event type field
     *
     * @param[in] type - The event type
     */
    void setType(uint8_t type);

    /**
     * @brief Set the action flags field
     *
     * @param[in] action - The action flags to set
     */
    void setAction(uint16_t action);

  private:
    /**
     * @brief The subsystem associated with the event.
     */
    uint8_t _eventSubsystem;

    /**
     * @brief The event scope field.
     */
    uint8_t _eventScope = static_cast<uint8_t>(EventScope::platform);

    /**
     * @brief The event severity.
     */
    uint8_t _eventSeverity; // set by constructor

    /**
     * @brief The event type.
     */
    uint8_t _eventType = static_cast<uint8_t>(EventType::trace);

    /**
     * @brief A reserved 4 byte placeholder
     */
    uint32_t _reserved4Byte1 = 0;

    /**
     * @brief The problem domain field.
     */
    uint8_t _problemDomain = 0;

    /**
     * @brief The problem vector field.
     */
    uint8_t _problemVector = 0;

    /**
     * @brief The action flags field.
     */
    uint16_t _actionFlags; // set by contructor

    /**
     * @brief A reserved 4 byte
     * placeholder
     */
    uint32_t _reserved4Byte2 = 0;
};

} // namespace pel
} // namespace attn
