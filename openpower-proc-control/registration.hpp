#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <string>

namespace openpower
{
namespace util
{

using ProcedureName = std::string;
using ProcedureFunction = std::function<void()>;
using ProcedureMap = std::map<ProcedureName, ProcedureFunction>;

/**
 * This macro can be used in each procedure cpp file to make it
 * available to the openpower-proc-control executable.
 */
#define REGISTER_PROCEDURE(name, func)                                         \
    namespace func##_ns                                                        \
    {                                                                          \
        openpower::util::Registration r{std::move(name), std::move(func)};     \
    }

/**
 * Used to register procedures.  Each procedure function can then
 * be found in a map via its name.
 */
class Registration
{
  public:
    /**
     *  Adds the procedure name and function to the internal
     *  procedure map.
     *
     *  @param[in] name - the procedure name
     *  @param[in] function - the function to run
     */
    Registration(ProcedureName&& name, ProcedureFunction&& function)
    {
        procedures.emplace(std::move(name), std::move(function));
    }

    /**
     * Returns the map of procedures
     */
    static const ProcedureMap& getProcedures()
    {
        return procedures;
    }

  private:
    static ProcedureMap procedures;
};

} // namespace util
} // namespace openpower
