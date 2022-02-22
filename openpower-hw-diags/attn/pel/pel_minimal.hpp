#pragma once

#include "extended_user_header.hpp"
#include "pel_common.hpp"
#include "primary_src.hpp"
#include "private_header.hpp"
#include "user_header.hpp"

#include <vector>

namespace attn
{
namespace pel
{

/** @class PelMinimal
 *
 * @brief Class for a minimal platform event log (PEL)
 *
 * This class can be used to create form a PEL and create a raw PEL file. The
 * implementation based on "Platform Event Log and SRC PLDD v1.1"
 *
 * This PEL consists of the following position dependent sections:
 *
 * |----------+------------------------------|
 * | length   | section                      |
 * |----------+------------------------------|
 * | 48       | Private Header Section       |
 * |----------+------------------------------|
 * | 24       | User Header Section          |
 * |----------+------------------------------|
 * | 72       | Primary SRC Section          |
 * |----------+------------------------------|
 * | 20       | Extended User Header         |
 * |----------+------------------------------|
 */
class PelMinimal
{
  public:
    PelMinimal()                  = delete;
    ~PelMinimal()                 = default;
    PelMinimal(const PelMinimal&) = delete;
    PelMinimal& operator=(const PelMinimal&) = delete;
    PelMinimal(PelMinimal&&)                 = delete;
    PelMinimal& operator=(PelMinimal&&) = delete;

    /**
     * @brief Create a minimal PEL object from raw data
     *
     * @param[in] pelBuffer - buffer containing a raw PEL
     */
    explicit PelMinimal(std::vector<uint8_t>& data);

    /**
     * @brief Initialize the object's data members
     *
     * @param[in] data - reference to the vector
     */
    void initialize(std::vector<uint8_t>& data);

    /**
     * @brief Stream raw PEL data to buffer
     *
     * @param[out] pelBuffer - What the data will be written to
     */
    void raw(std::vector<uint8_t>& pelBuffer) const;

    /**
     * @brief Set the User Header subsystem field
     *
     * @param[in] subsystem - The subsystem value
     */
    void setSubsystem(uint8_t subsystem);

    /**
     * @brief Set the User Header severity field
     *
     * @param[in] severity - The severity to set
     */
    void setSeverity(uint8_t severity);

    /**
     * @brief Set the User Header event type field
     *
     * @param[in] type - The event type
     */
    void setType(uint8_t type);

    /**
     * @brief Set the User Header action flags field
     *
     * @param[in] action - The action flags to set
     */
    void setAction(uint16_t action);

    /**
     * @brief Set the Primary SRC section SRC words
     *
     * @param[in] srcWords - The SRC words
     */
    void setSrcWords(std::array<uint32_t, numSrcWords> srcWords);

    /**
     * @brief Set the Primary SRC section ascii string field
     *
     * @param[in]  asciiString - The ascii string
     */
    void setAsciiString(std::array<char, asciiStringSize> asciiString);

    /**
     * @brief Get section count from the private header
     *
     * @return Number of sections
     */
    uint8_t getSectionCount();

    /**
     * @brief Set section count in private heasder
     *
     * @param[in] sectionCount - Number of sections
     */
    void setSectionCount(uint8_t sectionCount);

    /**
     * @brief Set the symptom id field in extended user header
     *
     * @param[in] symptomId - The symptom ID to set
     */
    void setSymptomId(const std::string& symptomId);

    /**
     * @brief Update the PLID
     */
    void setPlid(uint32_t plid);

  private:
    /**
     * @brief Maximum PEL size
     */
    static constexpr size_t _maxPELSize = 16384;

    /**
     * @brief Returns the size of the PEL
     *
     * @return size_t The PEL size in bytes
     */
    size_t size() const;

    /**
     * @brief PEL Private Header
     */
    std::unique_ptr<PrivateHeader> _ph;

    /**
     * @brief PEL User Header
     */
    std::unique_ptr<UserHeader> _uh;

    /**
     * @brief PEL Primary SRC
     */
    std::unique_ptr<PrimarySrc> _ps;

    /**
     * @brief PEL Extended User Header
     */
    std::unique_ptr<ExtendedUserHeader> _eh;
};

} // namespace pel
} // namespace attn
