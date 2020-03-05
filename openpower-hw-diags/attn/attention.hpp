#pragma once

#include <libpdbg.h>

namespace attn
{

/** @brief attention handler configuration flags */
inline constexpr uint32_t enableBreakpoints = 1;

/**
 * @brief These objects contain information about an active attention.
 *
 * An Attention object is created for each active attention. These objects
 * carry with them various configuration and status information as well
 * the attention handler function to call for handling the attention. Each
 * Attention object also carries a priority value. This priority is used
 * to determine which attention event(s) to handle when there are more than
 * one active event.
 */
class Attention
{
  public:
    /** @brief types of attentions to be handled (by priority low to high) */
    enum AttentionType
    {
        Special   = 0,
        Checkstop = 1,
        Vital     = 2
    };

    /** @brief Default constructor. */
    Attention() = delete;

    /** @brief Main constructors */
    Attention(AttentionType i_type, int (*i_handler)(Attention*),
              pdbg_target* i_target, bool i_breakpoints);

    /** @brief Destructor */
    ~Attention() = default;

    /** @brief Get attention priority */
    int getPriority() const;

    /** @brief Get configuration flags */
    uint32_t getFlags() const;

    /** @brief Set configuration flags */
    void setFlags(uint32_t i_flags);

    /* @brief Call attention handler function */
    int handle();

    /** @brief Copy constructor. */
    Attention(const Attention&) = default;

    /** @brief Assignment operator. */
    Attention& operator=(const Attention&) = default;

    /** @brief less than operator */
    bool operator<(const Attention& right) const;

  private:
    AttentionType iv_type;         // attention type
    int (*iv_handler)(Attention*); // handler function
    pdbg_target* iv_target;        // handler function target
    uint32_t iv_flags = 0;         // configuration flags
};

} // namespace attn
