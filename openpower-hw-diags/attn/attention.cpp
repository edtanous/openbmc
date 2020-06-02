#include <attention.hpp>
#include <attn_config.hpp>

namespace attn
{

/** @brief Main constructor. */
Attention::Attention(AttentionType i_type, int (*i_handler)(Attention*),
                     pdbg_target* i_target, Config* i_config) :
    iv_type(i_type),
    iv_handler(i_handler), iv_target(i_target), iv_config(i_config)

{}

/** @brief Get attention priority */
int Attention::getPriority() const
{
    return iv_type;
}

/* @brief Get config object */
Config* Attention::getConfig() const
{
    return iv_config;
}

/* @brief Call attention handler function */
int Attention::handle()
{
    return iv_handler(this);
}

/* @brief Get attention handler target */
pdbg_target* Attention::getTarget() const
{
    return iv_target;
}

/** @brief less than operator, for heap creation */
bool Attention::operator<(const Attention& right) const
{
    return (getPriority() < right.getPriority());
}

} // namespace attn
