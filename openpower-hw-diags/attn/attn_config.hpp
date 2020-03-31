#pragma once
#include <bitset>

namespace attn
{

/** @brief configuration flags */
enum AttentionFlag
{
    enVital       = 0,
    enCheckstop   = 1,
    enTerminate   = 2,
    enBreakpoints = 3,
    lastFlag
};

/** @brief Objhects to hold configuration data */
class Config
{
  public: // methods
    /** @brief Default constructor */
    Config() = delete;

    /** @brief Crate configuration object
     *
     * Create a configuration object to hold the attention handler
     * configuration data
     *
     * @param i_vital       Enable vital attention handling
     * @param i_checkstop   Enable checkstop attention handling
     * @param i_terminate   Enable TI attention handling
     * @param i_+breakpoint Enable breakpoint attention handling
     */
    Config(bool i_vital, bool i_checkstop, bool i_terminate,
           bool i_breakpoints);

    /** @brief Default destructor */
    ~Config() = default;

    /** @brief Get state of flag */
    bool getFlag(AttentionFlag i_flag) const;

    /** @brief Set configuration flag */
    void setFlag(AttentionFlag i_flag);

    /** @brief Clear configuration flag */
    void clearFlag(AttentionFlag i_flag);

    /** @brief Set state of all configuration data */
    void setConfig(bool i_vital, bool i_checkstop, bool i_terminate,
                   bool i_breakpoints);

  private:
    std::bitset<lastFlag> iv_flags; // configuration flags
};

} // namespace attn
