#pragma once

#include <nlohmann/json.hpp>

namespace analyzer
{

/** @brief An abstract class for service event, also known as a callout. */
class Callout
{
  public:
    /** Each callout will have a priority indicating when to issue the required
     *  service action. Details below. */
    enum Priority
    {
        /** Serivce is mandatory. */
        HIGH,

        /** Serivce medium priority callouts one at a time, in order, until the
         *  issue is resolved. */
        MED,

        MED_A, ///< Same as PRI_MED, except replace all A's as a group.
        MED_B, ///< Same as PRI_MED, except replace all B's as a group.
        MED_C, ///< Same as PRI_MED, except replace all C's as a group.

        /** If servicing all high and medium priority callouts did not resolve
         *  the issue, service low priority callouts one at a time, in order,
         *  until the issue is resolved. */
        LOW,
    };

  public:
    /** @brief Pure virtual destructor. */
    virtual ~Callout() = 0;

  protected:
    /**
     * @brief Constructor from components.
     * @param p The callout priority.
     */
    explicit Callout(Priority p) : iv_priority(p) {}

  private:
    /** The callout priority. */
    const Priority iv_priority;

  protected:
    /**
     * @brief Appends the callout priority to the end of the given json object.
     * @param j The json object for a single callout.
     */
    void addPriority(nlohmann::json& j) const
    {
        // clang-format off
        static const std::map<Priority, std::string> m =
        {
            {HIGH,  "H"},
            {MED,   "M"},
            {MED_A, "A"},
            {MED_B, "B"},
            {MED_C, "C"},
            {LOW,   "L"},
        };
        // clang-format on

        j.emplace("Priority", m.at(iv_priority));
    }

  public:
    /**
     * @brief Appends a json object representing this callout to the end of the
     *        given json object.
     * @param j The json object containing all current callouts for a PEL.
     */
    virtual void getJson(nlohmann::json&) const = 0;
};

// Pure virtual destructor must be defined.
inline Callout::~Callout() {}

/** @brief A service event requiring hardware replacement. */
class HardwareCallout : public Callout
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_locationCode The location code of the hardware callout.
     * @param i_priority     The callout priority.
     */
    HardwareCallout(const std::string& i_locationCode, Priority i_priority) :
        Callout(i_priority), iv_locationCode(i_locationCode)
    {}

  private:
    /** The hardware location code. */
    const std::string iv_locationCode;

  public:
    void getJson(nlohmann::json& j) const override
    {
        nlohmann::json c = {{"LocationCode", iv_locationCode}};
        addPriority(c);
        j.emplace_back(c);
    }
};

/**
 * @brief A service event requiring a special procedure to be handled by a
 *        service engineer.
 */
class ProcedureCallout : public Callout
{
  public:
    /** Supported service procedures. */
    enum Procedure
    {
        NEXTLVL, ///< Contact next level support.
    };

  public:
    /**
     * @brief Constructor from components.
     * @param i_procedure The location code of the hardware callout.
     * @param i_priority     The callout priority.
     */
    ProcedureCallout(Procedure i_procedure, Priority i_priority) :
        Callout(i_priority), iv_procedure(i_procedure)
    {}

  private:
    /** The callout priority. */
    const Procedure iv_procedure;

  public:
    void getJson(nlohmann::json& j) const override
    {
        // clang-format off
        static const std::map<Procedure, std::string> m =
        {
            {NEXTLVL, "NEXTLVL"},
        };
        // clang-format on

        nlohmann::json c = {{"Procedure", m.at(iv_procedure)}};
        addPriority(c);
        j.emplace_back(c);
    }
};

} // namespace analyzer
