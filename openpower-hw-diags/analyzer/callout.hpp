#pragma once

#include <string>

namespace analyzer
{

namespace callout
{

/** @brief All callouts will have a priority indicating when to issue the
 *         required service action. */
class Priority
{
  public:
    /** Serivce is mandatory. */
    static const Priority HIGH;

    /** Serivce medium priority callouts one at a time, in order, until the
     *  issue is resolved. */
    static const Priority MED;

    /** Same as MED, except replace all A's as a group. */
    static const Priority MED_A;

    /** Same as MED, except replace all A's as a group. */
    static const Priority MED_B;

    /** Same as MED, except replace all A's as a group. */
    static const Priority MED_C;

    /** If servicing all high and medium priority callouts did not resolve
     *  the issue, service low priority callouts one at a time, in order,
     *  until the issue is resolved. */
    static const Priority LOW;

  private:
    /**
     * @brief Constructor from components.
     *
     * At the moment, the priority values for the callout list user data
     * section are different from the priority values hard-coded in the
     * registry. Therefore, two different values must be stored.
     *
     * @param i_registry The string representation of a priority used in
     *                   registry callouts.
     * @param i_userData The string representation of a priority used in user
     *                   data callouts.
     */
    Priority(const std::string& i_registry, const std::string& i_userData) :
        iv_registry(i_registry), iv_userData(i_userData)
    {}

  private:
    /** The string representation of a priority used in registry callouts. */
    const std::string iv_registry;

    /** The string representation of a priority used in user data callouts. */
    const std::string iv_userData;

  public:
    /** iv_registry accessor */
    const std::string& getRegistryString() const
    {
        return iv_registry;
    }

    /** iv_userData accessor */
    const std::string& getUserDataString() const
    {
        return iv_userData;
    }
};

// clang-format off
inline const Priority Priority::HIGH {"high",          "H"};
inline const Priority Priority::MED  {"medium",        "M"};
inline const Priority Priority::MED_A{"medium_group_A","A"};
inline const Priority Priority::MED_B{"medium_group_B","B"};
inline const Priority Priority::MED_C{"medium_group_C","C"};
inline const Priority Priority::LOW  {"low",           "L"};
// clang-format on

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
