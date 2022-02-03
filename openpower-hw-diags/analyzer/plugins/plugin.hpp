#pragma once

#include <string.h>

#include <analyzer/service_data.hpp>
#include <hei_chip.hpp>
#include <util/trace.hpp>

#include <functional>
#include <map>

namespace analyzer
{

// A plugin is special function called by the RAS data files specifically for
// service actions that cannot be characterized by the RAS data files.
//
// IMPORTANT:
//   Use of these plugins should be limited to avoid maintaining chip specific
//   functionality in this repository.
//
// Each plugin must be defined in a specific form. See PluginFunction below.
// Then the plugin must registered in the PluginMap using the PLUGIN_DEFINE or
// PLUGIN_DEFINE_NS macros below.

// All plugins will have the following parameters:
//  - The plugin instance for plugins that may be defined for a chip that has
//    multiple instance of a unit/register.
//  - The chip containing the root cause attention.
//  - The service data object containing service actions and FFDC for the root
//    cause attention.
using PluginFunction =
    std::function<void(unsigned int, const libhei::Chip&, ServiceData&)>;

// These are provided as know chip types for plugin definitions.
constexpr libhei::ChipType_t EXPLORER_11 = 0x60d20011;
constexpr libhei::ChipType_t EXPLORER_20 = 0x60d20020;
constexpr libhei::ChipType_t P10_10      = 0x20da0010;
constexpr libhei::ChipType_t P10_20      = 0x20da0020;

/**
 * @brief This is simply a global container for all of the registered plugins.
 *
 * @note  This class cannot be instantiated. Instead, use the getSingleton()
 *        function to access.
 */
class PluginMap
{
  private:
    /** @brief Default constructor. */
    PluginMap() = default;

    /** @brief Destructor. */
    ~PluginMap() = default;

    /** @brief Copy constructor. */
    PluginMap(const PluginMap&) = delete;

    /** @brief Assignment operator. */
    PluginMap& operator=(const PluginMap&) = delete;

  public:
    /** @brief Provides access to a singleton instance of this object. */
    static PluginMap& getSingleton()
    {
        static PluginMap thePluginMap;
        return thePluginMap;
    }

  private:
    /** A nested map that contains the function for each chip type and plugin
     *  name. */
    std::map<libhei::ChipType_t, std::map<std::string, PluginFunction>> iv_map;

  public:
    /**
     * @brief Registers a plugin with the plugin map.
     *
     * @param i_type   The chip type associated with the plugin.
     * @param i_name   The name of the plugin.
     * @param i_plugin The plugin function.
     *
     * @throw std::logic_error if a plugin is defined more than once.
     */
    void add(libhei::ChipType_t i_type, const std::string& i_name,
             PluginFunction i_plugin)
    {
        auto itr = iv_map.find(i_type);
        if (iv_map.end() == itr ||
            itr->second.end() == itr->second.find(i_name))
        {
            iv_map[i_type][i_name] = i_plugin;
        }
        else
        {
            throw std::logic_error("Duplicate plugin found");
        }
    }

    /**
     * @return The plugin function for the target plugin.
     *
     * @param i_type The chip type associated with the plugin.
     * @param i_name The name of the plugin.
     *
     * @throw std::out_of_range if the target plugin does not exist.
     */
    PluginFunction get(libhei::ChipType_t i_type,
                       const std::string& i_name) const
    {
        PluginFunction func;

        try
        {
            func = iv_map.at(i_type).at(i_name);
        }
        catch (const std::out_of_range& e)
        {
            trace::err("Plugin not defined: i_type=0x%08x i_name=%s", i_type,
                       i_name.c_str());
            throw; // caught later downstream
        }

        return func;
    }
};

// These defines a unique class and a global variable for each plugin. Because
// the variables are defined in the global scope, they will be initialized with
// the default constructor before execution of the program. This allows all of
// the plugins to be registered in the plugin map before execution.

#define __PLUGIN_MAKE(X, Y, Z) X##Y##Z

#define __PLUGIN_DEFINE(CHIP, NAME, FUNC)                                      \
    class __PLUGIN_MAKE(Plugin_, CHIP, NAME)                                   \
    {                                                                          \
      public:                                                                  \
        __PLUGIN_MAKE(Plugin_, CHIP, NAME)                                     \
        ()                                                                     \
        {                                                                      \
            PluginMap::getSingleton().add(CHIP, #NAME, &FUNC);                 \
        }                                                                      \
    };                                                                         \
    __PLUGIN_MAKE(Plugin_, CHIP, NAME) __PLUGIN_MAKE(g_Plugin_, CHIP, NAME)

#define PLUGIN_DEFINE(CHIP, NAME) __PLUGIN_DEFINE(CHIP, NAME, CHIP::NAME)

#define PLUGIN_DEFINE_NS(CHIP, NS, NAME) __PLUGIN_DEFINE(CHIP, NAME, NS::NAME)

// Regarding the use of PLUGIN_DEFINE_NS. This is provided for cases where the
// same plugin needs to be defined differently for different chips. Example:
//
//    namespace A
//    {
//        void foo(...) { /* definition for chip A */ }
//    };
//
//    namespace B
//    {
//        void foo(...) { /* definition for chip B */ }
//    };
//
//    PLUGIN_DEFINE_NS(CHIP_A, A, foo);
//    PLUGIN_DEFINE_NS(CHIP_B, B, foo);
//
// Also, it is important that the plugin definitions should be declared outside
// of the function namespaces (see the example above). This helps find
// duplicate plugin definitions at compile time. Otherwise, if you do something
// like this:
//
//    namespace A
//    {
//        void foo(...) { /* definition for chip A */ }
//        PLUGIN_DEFINE_NS(CHIP_A, A, foo);
//    };
//
//    namespace B
//    {
//        void foo(...) { /* definition again for chip A */ }
//        PLUGIN_DEFINE_NS(CHIP_A, B, foo);
//    };
//
// The compiler will not find the duplicate and instead it will be found during
// program execution, which will result in an exception.

} // namespace analyzer
