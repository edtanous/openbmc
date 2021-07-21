#include <util/dbus.hpp>
#include <util/trace.hpp>

namespace util
{

namespace dbus
{

//------------------------------------------------------------------------------

constexpr auto objectMapperService   = "xyz.openbmc_project.ObjectMapper";
constexpr auto objectMapperPath      = "/xyz/openbmc_project/object_mapper";
constexpr auto objectMapperInterface = "xyz.openbmc_project.ObjectMapper";

/** @brief Find the path and service that implements the given interface */
int find(const std::string& i_interface, std::string& o_path,
         std::string& o_service)
{
    int rc = 1; // assume not success

    auto bus = sdbusplus::bus::new_default();

    try
    {
        constexpr auto function = "GetSubTree";

        auto method = bus.new_method_call(objectMapperService, objectMapperPath,
                                          objectMapperInterface, function);

        // Search the entire dbus tree for the specified interface
        method.append(std::string{"/"}, 0,
                      std::vector<std::string>{i_interface});

        auto reply = bus.call(method);

        DBusSubTree response;
        reply.read(response);

        if (!response.empty())
        {
            // Response is a map of object paths to a map of service, interfaces
            auto object = *(response.begin());
            o_path      = object.first;                 // return path
            o_service   = object.second.begin()->first; // return service

            rc = 0; // success
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace::err("util::dbus::find exception");
        std::string traceMsg = std::string(e.what());
        trace::err(traceMsg.c_str());
    }

    return rc;
}

/** @brief Find the service that implements the given object and interface */
int findService(const std::string& i_interface, const std::string& i_path,
                std::string& o_service)
{
    int rc = 1; // assume not success

    auto bus = sdbusplus::bus::new_default();

    try
    {
        constexpr auto function = "GetObject";

        auto method = bus.new_method_call(objectMapperService, objectMapperPath,
                                          objectMapperInterface, function);

        // Find services that implement the object path, constrain the search
        // to the given interface.
        method.append(i_path, std::vector<std::string>{i_interface});

        auto reply = bus.call(method);

        // response is a map of service names to their interfaces
        std::map<DBusService, DBusInterfaceList> response;
        reply.read(response);

        if (!response.empty())
        {
            // return the service
            o_service = response.begin()->first;

            rc = 0; // success
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace::err("util::dbus::map exception");
        std::string traceMsg = std::string(e.what());
        trace::err(traceMsg.c_str());
    }

    return rc;
}

/** @brief Read a property from a dbus object interface */
int getProperty(const std::string& i_interface, const std::string& i_path,
                const std::string& i_service, const std::string& i_property,
                DBusValue& o_response)
{
    int rc = 1; // assume not success

    auto bus = sdbusplus::bus::new_default();

    try
    {
        constexpr auto interface = "org.freedesktop.DBus.Properties";
        constexpr auto function  = "Get";

        // calling the get property method
        auto method = bus.new_method_call(i_service.c_str(), i_path.c_str(),
                                          interface, function);

        method.append(i_interface, i_property);
        auto reply = bus.call(method);

        // returning the property value
        reply.read(o_response);

        rc = 0; // success
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace::err("util::dbus::getProperty exception");
        std::string traceMsg = std::string(e.what());
        trace::err(traceMsg.c_str());
    }

    return rc;
}

/** @brief Get the IBM compatible names defined for this system */
std::vector<std::string> systemNames()
{
    std::vector<std::string> names;

    constexpr auto interface =
        "xyz.openbmc_project.Configuration.IBMCompatibleSystem";

    DBusService service;
    DBusPath path;

    // find a dbus object and path that implements the interface
    if (0 == find(interface, path, service))
    {
        DBusValue value;

        // compatible system names are implemented as a property
        constexpr auto property = "Names";

        if (0 == getProperty(interface, path, service, property, value))
        {
            // return value is a variant, names are in the vector
            names = std::get<std::vector<std::string>>(value);
        }
    }

    return names;
}

} // namespace dbus

} // namespace util
