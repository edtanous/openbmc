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

/** These SRC subsystem values are defined by the PEL spec. */
enum class SrcSubsystem
{
    // Can also reference `extensions/openpower-pels/pel_values.cpp` from
    // `openbmc/phosphor-logging` for the full list of these values.

    PROCESSOR      = 0x10,
    PROCESSOR_FRU  = 0x11,
    PROCESSOR_UNIT = 0x13,
    PROCESSOR_BUS  = 0x14,

    MEMORY_CTLR = 0x21,
    MEMORY_BUS  = 0x22,
    MEMORY_DIMM = 0x23,
    MEMORY_FRU  = 0x24,

    PHB = 0x38,

    CEC_HARDWARE = 0x50,
    CEC_CLOCKS   = 0x58,
    CEC_TOD      = 0x5A,

    OTHERS = 0x70,
};

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
    Procedure(const std::string& i_string, const SrcSubsystem i_subsystem) :
        iv_string(i_string), iv_subsystem(i_subsystem)
    {}

  private:
    /** The string representation of the procedure used for callouts. */
    const std::string iv_string;

    /** The associated SRC subsystem of the procedure. */
    const SrcSubsystem iv_subsystem;

  public:
    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }

    /** iv_subsystem accessor */
    SrcSubsystem getSrcSubsystem() const
    {
        return iv_subsystem;
    }
};

inline const Procedure Procedure::NEXTLVL{"next_level_support",
                                          SrcSubsystem::OTHERS};

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
    BusType(const std::string& i_string, const SrcSubsystem i_subsystem) :
        iv_string(i_string), iv_subsystem(i_subsystem)
    {}

  private:
    /** The string representation of the procedure used for callouts. */
    const std::string iv_string;

    /** The associated SRC subsystem of the bus. */
    const SrcSubsystem iv_subsystem;

  public:
    bool operator==(const BusType& r) const
    {
        return ((this->iv_string == r.iv_string) &&
                (this->iv_subsystem == r.iv_subsystem));
    }

    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }

    /** iv_subsystem accessor */
    SrcSubsystem getSrcSubsystem() const
    {
        return iv_subsystem;
    }
};

inline const BusType BusType::SMP_BUS{"SMP_BUS", SrcSubsystem::PROCESSOR_BUS};
inline const BusType BusType::OMI_BUS{"OMI_BUS", SrcSubsystem::MEMORY_BUS};

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
    ClockType(const std::string& i_string, const SrcSubsystem i_subsystem) :
        iv_string(i_string), iv_subsystem(i_subsystem)
    {}

  private:
    /** The string representation of the procedure used for callouts. */
    const std::string iv_string;

    /** The associated SRC subsystem of the clock. */
    const SrcSubsystem iv_subsystem;

  public:
    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }

    /** iv_subsystem accessor */
    SrcSubsystem getSrcSubsystem() const
    {
        return iv_subsystem;
    }
};

inline const ClockType ClockType::OSC_REF_CLOCK_0{"OSC_REF_CLOCK_0",
                                                  SrcSubsystem::CEC_CLOCKS};
inline const ClockType ClockType::OSC_REF_CLOCK_1{"OSC_REF_CLOCK_1",
                                                  SrcSubsystem::CEC_CLOCKS};
inline const ClockType ClockType::TOD_CLOCK{"TOD_CLOCK", SrcSubsystem::CEC_TOD};

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
    PartType(const std::string& i_string, const SrcSubsystem i_subsystem) :
        iv_string(i_string), iv_subsystem(i_subsystem)
    {}

  private:
    /** The string representation of the part callout. */
    const std::string iv_string;

    /** The associated SRC subsystem of the part. */
    const SrcSubsystem iv_subsystem;

  public:
    bool operator==(const PartType& r) const
    {
        return ((this->iv_string == r.iv_string) &&
                (this->iv_subsystem == r.iv_subsystem));
    }

    /** iv_string accessor */
    std::string getString() const
    {
        return iv_string;
    }

    /** iv_subsystem accessor */
    SrcSubsystem getSrcSubsystem() const
    {
        return iv_subsystem;
    }
};

inline const PartType PartType::PNOR{"PNOR", SrcSubsystem::CEC_HARDWARE};

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
