#pragma once

#include <analyzer/callout.hpp>
#include <hei_main.hpp>
#include <nlohmann/json.hpp>

namespace analyzer
{

/**
 * @brief A service event requiring hardware to be guarded.
 */
class Guard
{
  public:
    /** Supported guard types. */
    enum Type
    {
        NONE,      ///< Do not guard
        FATAL,     ///< Guard on fatal error (cannot recover resource)
        NON_FATAL, ///< Guard on non-fatal error (can recover resource)
    };

  public:
    /**
     * @brief Constructor from components.
     * @param i_path The hardware path to guard.
     * @param i_type The guard type.
     */
    Guard(const std::string& i_path, Type i_type) :
        iv_path(i_path), iv_type(i_type)
    {}

  private:
    /** The hardware path to guard. */
    const std::string iv_path;

    /** The guard type. */
    const Type iv_type;

  public:
    void getJson(nlohmann::json& j) const
    {
        // clang-format off
        static const std::map<Type, std::string> m =
        {
            {NONE,      "NONE"},
            {FATAL,     "FATAL"},
            {NON_FATAL, "NON_FATAL"},
        };
        // clang-format on

        nlohmann::json c = {{"Path", iv_path}, {"Type", m.at(iv_type)}};
        j.emplace_back(c);
    }
};

/**
 * @brief Data regarding required service actions based on the hardware error
 *        analysis.
 */
class ServiceData
{
  public:
    /**
     * @brief Constructor from components.
     * @param The signature of the root cause attention.
     * @param True if the signature list contained a system checkstop attention.
     *        False, otherwise.
     */
    ServiceData(const libhei::Signature& i_rootCause, bool i_isCheckstop) :
        iv_rootCause(i_rootCause), iv_isCheckstop(i_isCheckstop)
    {}

    /** @brief Destructor. */
    ~ServiceData() = default;

    /** @brief Copy constructor. */
    ServiceData(const ServiceData&) = default;

    /** @brief Assignment operator. */
    ServiceData& operator=(const ServiceData&) = default;

  private:
    /** The signature of the root cause attention. */
    const libhei::Signature iv_rootCause;

    /** True if the signature list contained a system checkstop attention.
     *  False, otherwise. */
    const bool iv_isCheckstop;

    /** The list of callouts that will be added to a PEL. */
    nlohmann::json iv_calloutList = nlohmann::json::array();

    /** The list of hardware guard requests. Some information will be added to
     * the PEL, but the actual guard record will be created after submitting the
     * PEL. */
    std::vector<std::shared_ptr<Guard>> iv_guardList;

  public:
    /** @return The signature of the root cause attention. */
    const libhei::Signature& getRootCause() const
    {
        return iv_rootCause;
    }

    /** @return True if the signature list contained a system checkstop
     *          attention. False, otherwise. */
    bool queryCheckstop() const
    {
        return iv_isCheckstop;
    }

    /**
     * @brief Add callout information to the callout list.
     * @param The JSON object for this callout.
     */
    void addCallout(const nlohmann::json& i_callout)
    {
        iv_calloutList.push_back(i_callout);
    }

    /** Add a guard request to the list. */
    void addGuard(const std::shared_ptr<Guard>& i_guard)
    {
        iv_guardList.push_back(i_guard);
    }

    /** @brief Accessor to iv_calloutList. */
    const nlohmann::json& getCalloutList() const
    {
        return iv_calloutList;
    }

    /**
     * @brief Iterates the guard list and returns the json attached to each
     *        guard request in the list.
     * @param o_json The returned json data.
     */
    void getGuardList(nlohmann::json& o_json) const
    {
        o_json.clear(); // Ensure we are starting with a clean list.

        for (const auto& g : iv_guardList)
        {
            g->getJson(o_json);
        }
    }
};

} // namespace analyzer
