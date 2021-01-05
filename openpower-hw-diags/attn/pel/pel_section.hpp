#pragma once

#include "section_header.hpp"

namespace attn
{
namespace pel
{
/**
 * @class Section
 *
 * The base class for a PEL section.  It contains the SectionHeader
 * as all sections start with it.
 *
 */
class Section
{
  public:
    Section()               = default;
    virtual ~Section()      = default;
    Section(const Section&) = default;
    Section& operator=(const Section&) = default;
    Section(Section&&)                 = default;
    Section& operator=(Section&&) = default;

    /**
     * @brief Returns a reference to the SectionHeader
     */
    const SectionHeader& header() const
    {
        return _header;
    }

    /**
     * @brief Flatten the section into the stream
     *
     * @param[in] stream - The stream to write to
     */
    virtual void flatten(Stream& stream) const = 0;

  protected:
    /**
     * @brief Returns the flattened size of the section header
     */
    static constexpr size_t flattenedSize()
    {
        return SectionHeader::flattenedSize();
    }

    /**
     * @brief The section header structure.
     *
     * Filled in by derived classes.
     */
    SectionHeader _header;
};

} // namespace pel
} // namespace attn
