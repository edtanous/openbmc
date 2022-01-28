#pragma once

#include <map>
#include <string>

namespace analyzer
{

namespace callout
{

/** @brief All callouts will have a priority indicating when to issue the
 *         required service action. */
enum class Priority
{
    /** Serivce is mandatory. */
    HIGH,

    /** Serivce medium priority callouts one at a time, in order, until the
     *  issue is resolved. */
    MED,

    /** Same as MED, except replace all A's as a group. */
    MED_A,

    /** Same as MED, except replace all B's as a group. */
    MED_B,

    /** Same as MED, except replace all C's as a group. */
    MED_C,

    /** If servicing all high and medium priority callouts did not resolve
     *  the issue, service low priority callouts one at a time, in order,
     *  until the issue is resolved. */
    LOW,
};

/** @return The string representation of the priority used in callouts. */
inline std::string getString(Priority i_priority)
{
    // clang-format off
    static const std::map<Priority, std::string> m =
    {
        {Priority::HIGH,  "H"},
        {Priority::MED,   "M"},
        {Priority::MED_A, "A"},
        {Priority::MED_B, "B"},
        {Priority::MED_C, "C"},
        {Priority::LOW,   "L"},
    };
    // clang-format on

    return m.at(i_priority);
}

/** @return The string representation of the priority used in The callout FFDC.
 */
inline std::string getStringFFDC(Priority i_priority)
{
    // clang-format off
    static const std::map<Priority, std::string> m =
    {
        {Priority::HIGH,  "high"},
        {Priority::MED,   "medium"},
        {Priority::MED_A, "medium_group_A"},
        {Priority::MED_B, "medium_group_B"},
        {Priority::MED_C, "medium_group_C"},
        {Priority::LOW,   "low"},
    };
    // clang-format on

    return m.at(i_priority);
}

/** @brief Container class for procedure callout service actions. */
class Procedure
{
  public:
    /** Contact next level support. */
    static const Procedure NEXTLVL;

  private:
    /**
     * @brief Constructor from components.
     * @param i_string The string representation of the procedure used for
     *                 callouts.
     */
    explicit Procedure(const std::string& i_string) : iv_string(i_string) {}

  private:
    /** The string representation of the procedure used for callouts. */
    const std::string iv_string;

  public:
    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }
};

inline const Procedure Procedure::NEXTLVL{"next_level_support"};

/** @brief Container class for bus callout service actions. */
class BusType
{
  public:
    /** SMP bus (fabric A or X bus). */
    static const BusType SMP_BUS;

    /** OMI bus (memory bus). */
    static const BusType OMI_BUS;

  private:
    /**
     * @brief Constructor from components.
     * @param i_string The string representation of the procedure used for
     *                 callouts.
     */
    explicit BusType(const std::string& i_string) : iv_string(i_string) {}

  private:
    /** The string representation of the procedure used for callouts. */
    const std::string iv_string;

  public:
    bool operator==(const BusType& r) const
    {
        return this->iv_string == r.iv_string;
    }

    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }
};

inline const BusType BusType::SMP_BUS{"SMP_BUS"};
inline const BusType BusType::OMI_BUS{"OMI_BUS"};

/** @brief Container class for clock callout service actions. */
class ClockType
{
  public:
    /** Oscillator reference clock 0. */
    static const ClockType OSC_REF_CLOCK_0;

    /** Oscillator reference clock 1. */
    static const ClockType OSC_REF_CLOCK_1;

    /** Time of Day (TOD) clock. */
    static const ClockType TOD_CLOCK;

  private:
    /**
     * @brief Constructor from components.
     * @param i_string The string representation of the procedure used for
     *                 callouts.
     */
    explicit ClockType(const std::string& i_string) : iv_string(i_string) {}

  private:
    /** The string representation of the procedure used for callouts. */
    const std::string iv_string;

  public:
    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }
};

inline const ClockType ClockType::OSC_REF_CLOCK_0{"OSC_REF_CLOCK_0"};
inline const ClockType ClockType::OSC_REF_CLOCK_1{"OSC_REF_CLOCK_1"};
inline const ClockType ClockType::TOD_CLOCK{"TOD_CLOCK"};

/** @brief Container class for part callout service actions. */
class PartType
{
  public:
    /** The part containing the PNOR. */
    static const PartType PNOR;

  private:
    /**
     * @brief Constructor from components.
     * @param i_string The string representation of the part callout.
     */
    explicit PartType(const std::string& i_string) : iv_string(i_string) {}

  private:
    /** The string representation of the part callout. */
    const std::string iv_string;

  public:
    bool operator==(const PartType& r) const
    {
        return this->iv_string == r.iv_string;
    }

    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }
};

inline const PartType PartType::PNOR{"PNOR"};

/** @brief Container class for guard service actions. */
class GuardType
{
  public:
    /** Do not guard. */
    static const GuardType NONE;

    /** Guard on fatal error (cannot recover resource). */
    static const GuardType UNRECOVERABLE;

    /** Guard on non-fatal error (can recover resource). */
    static const GuardType PREDICTIVE;

  private:
    /**
     * @brief Constructor from components.
     * @param i_string The string representation of the procedure used for
     *                 callouts.
     */
    explicit GuardType(const std::string& i_string) : iv_string(i_string) {}

  private:
    /** The string representation of the procedure used for callouts. */
    const std::string iv_string;

  public:
    bool operator==(const GuardType& r) const
    {
        return this->iv_string == r.iv_string;
    }

    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }
};

inline const GuardType GuardType::NONE{"GARD_NULL"};
inline const GuardType GuardType::UNRECOVERABLE{"GARD_Unrecoverable"};
inline const GuardType GuardType::PREDICTIVE{"GARD_Predictive"};

} // namespace callout

} // namespace analyzer
