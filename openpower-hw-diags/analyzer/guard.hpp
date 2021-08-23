#pragma once

#include <map>
#include <string>

namespace analyzer
{

/**
 * @brief A service event requiring hardware to be guarded.
 */
class Guard
{
  public:
    /** Supported guard types. */
    enum class Type
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
    /** @brief Writes guard record to persistent storage. */
    void apply() const;

    /** @return A string representation of the guard type enum. */
    std::string getString() const
    {
        // clang-format off
        static const std::map<Type, std::string> m =
        {
            {Type::NONE,      "NONE"},
            {Type::FATAL,     "FATAL"},
            {Type::NON_FATAL, "NON_FATAL"},
        };
        // clang-format on

        return m.at(iv_type);
    }
};

} // namespace analyzer
