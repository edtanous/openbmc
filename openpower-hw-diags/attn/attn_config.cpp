#include <attn/attn_config.hpp>

namespace attn
{

/** @brief Main constructor */
Config::Config(bool i_vital, bool i_checkstop, bool i_terminate,
               bool i_breakpoints)
{
    setConfig(i_vital, i_checkstop, i_terminate, i_breakpoints);
}

/** @brief Get state of flag */
bool Config::getFlag(AttentionFlag i_flag) const
{
    return (iv_flags.test(i_flag));
}

/** @brief Set configuration flag */
void Config::setFlag(AttentionFlag i_flag)
{
    iv_flags.set(i_flag);
}

/** @brief Clear configuration flag */
void Config::clearFlag(AttentionFlag i_flag)
{
    iv_flags.reset(i_flag);
}

/** @brief Set state of all configuration data */
void Config::setConfig(bool i_vital, bool i_checkstop, bool i_terminate,
                       bool i_breakpoints)
{
    (true == i_vital) ? iv_flags.set(enVital) : iv_flags.reset(enVital);

    (true == i_checkstop) ? iv_flags.set(enCheckstop)
                          : iv_flags.reset(enCheckstop);

    (true == i_terminate) ? iv_flags.set(enTerminate)
                          : iv_flags.reset(enTerminate);

    (true == i_breakpoints) ? iv_flags.set(enBreakpoints)
                            : iv_flags.reset(enBreakpoints);
}

} // namespace attn
