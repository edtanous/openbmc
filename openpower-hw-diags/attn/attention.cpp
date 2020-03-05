#include <attention.hpp>

namespace attn
{

/** @brief Main constructor. */
Attention::Attention(AttentionType i_type, int (*i_handler)(Attention*),
                     pdbg_target* i_target, bool i_breakpoints) :
    iv_type(i_type),
    iv_handler(i_handler), iv_target(i_target)

{
    // set attention handler configuration flags
    if (true == i_breakpoints)
    {
        iv_flags |= enableBreakpoints;
    }
}

/** @brief Get attention priority */
int Attention::getPriority() const
{
    return iv_type;
}

/** @brief Get configuration flags */
uint32_t Attention::getFlags() const
{
    return iv_flags;
}

/** @brief Set configuration flags */
void Attention::setFlags(uint32_t i_flags)
{
    iv_flags = i_flags;
}

/* @brief Call attention handler function */
int Attention::handle()
{
    return iv_handler(this);
}

/** @brief less than operator */
bool Attention::operator<(const Attention& right) const
{
    return (getPriority() < right.getPriority());
}

} // namespace attn
