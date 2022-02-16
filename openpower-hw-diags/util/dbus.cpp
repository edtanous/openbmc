#include <util/dbus.hpp>
#include <util/trace.hpp>
#include <xyz/openbmc_project/State/Boot/Progress/server.hpp>

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

/** @brief Transition the host state */
void transitionHost(const HostState i_hostState)
{
    try
    {
        // We will be transitioning host by starting appropriate dbus target
        std::string target = "obmc-host-quiesce@0.target"; // quiesce is default

        // crash (mpipl) mode state requested
        if (HostState::Crash == i_hostState)
        {
            target = "obmc-host-crash@0.target";
        }

        auto bus    = sdbusplus::bus::new_system();
        auto method = bus.new_method_call(
            "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
            "org.freedesktop.systemd1.Manager", "StartUnit");

        method.append(target);    // target unit to start
        method.append("replace"); // mode = replace conflicting queued jobs

        bus.call_noreply(method); // start the service
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace::err("util::dbus::transitionHost exception");
        std::string traceMsg = std::string(e.what());
        trace::err(traceMsg.c_str());
    }
}

/** @brief Read state of autoRebootEnabled property via dbus */
bool autoRebootEnabled()
{
    // Assume true in case autoRebootEnbabled property is not available
    bool autoReboot = true;

    constexpr auto interface = "xyz.openbmc_project.Control.Boot.RebootPolicy";

    DBusService service; // will find this
    DBusPath path;       // will find this

    // find a dbus object and path that implements the interface
    if (0 == find(interface, path, service))
    {
        DBusValue value;

        // autoreboot policy is implemented as a property
        constexpr auto property = "AutoReboot";

        if (0 == getProperty(interface, path, service, property, value))
        {
            // return value is a variant, autoreboot policy is boolean
            autoReboot = std::get<bool>(value);
        }
    }

    return autoReboot;
}

/** @brief Get the running state of the host */
HostRunningState hostRunningState()
{
    // assume not able to get host running state
    HostRunningState host = HostRunningState::Unknown;

    constexpr auto interface = "xyz.openbmc_project.State.Boot.Progress";

    DBusService service;
    DBusPath path;

    // find a dbus object and path that implements the interface
    if (0 == find(interface, path, service))
    {
        DBusValue value;

        // boot progress is implemented as a property
        constexpr auto property = "BootProgress";

        if (0 == getProperty(interface, path, service, property, value))
        {
            // return value is a variant, progress is in the vector of strings
            std::string bootProgress(std::get<std::string>(value));

            // convert boot progress to host state
            using BootProgress = sdbusplus::xyz::openbmc_project::State::Boot::
                server::Progress::ProgressStages;

            BootProgress stage = sdbusplus::xyz::openbmc_project::State::Boot::
                server::Progress::convertProgressStagesFromString(bootProgress);

            if ((stage == BootProgress::SystemInitComplete) ||
                (stage == BootProgress::OSStart) ||
                (stage == BootProgress::OSRunning))
            {
                host = HostRunningState::Started;
            }
            else
            {
                host = HostRunningState::NotStarted;
            }
        }
    }

    return host;
}

/** @brief Read state of dumpPolicyEnabled property via dbus */
bool dumpPolicyEnabled()
{
    // Assume true In case dumpPolicyEnabled property is not available
    bool dumpPolicyEnabled = true;

    constexpr auto interface = "xyz.openbmc_project.Object.Enable";
    constexpr auto path      = "/xyz/openbmc_project/dump/system_dump_policy";

    DBusService service; // will find this

    // find a dbus object and path that implements the interface
    if (0 == findService(interface, path, service))
    {
        DBusValue value;

        // autoreboot policy is implemented as a property
        constexpr auto property = "Enabled";

        if (0 == getProperty(interface, path, service, property, value))
        {
            // return value is a variant, dump policy enabled is a boolean
            dumpPolicyEnabled = std::get<bool>(value);
        }
    }

    return dumpPolicyEnabled;
}

/** @brief Create a PEL */
uint32_t createPel(const std::string& i_message, const std::string& i_severity,
                   std::map<std::string, std::string>& io_additional,
                   const std::vector<util::FFDCTuple>& i_ffdc)
{
    // CreatePELWithFFDCFiles returns plid
    int plid = 0;

    // Sdbus call specifics
    constexpr auto interface = "org.open_power.Logging.PEL";
    constexpr auto path      = "/xyz/openbmc_project/logging";

    // we need to find the service implementing the interface
    util::dbus::DBusService service;

    if (0 == findService(interface, path, service))
    {
        try
        {
            constexpr auto function = "CreatePELWithFFDCFiles";

            // The "Create" method requires manually adding the process ID.
            io_additional["_PID"] = std::to_string(getpid());

            // create dbus method
            auto bus = sdbusplus::bus::new_system();
            sdbusplus::message::message method =
                bus.new_method_call(service.c_str(), path, interface, function);

            // append additional dbus call paramaters
            method.append(i_message, i_severity, io_additional, i_ffdc);

            // using system dbus
            auto response = bus.call(method);

            // reply will be tuple containing bmc log id, platform log id
            std::tuple<uint32_t, uint32_t> reply = {0, 0};

            // parse dbus response into reply
            response.read(reply);
            plid = std::get<1>(reply); // platform log id is tuple "second"
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            trace::err("createPel exception");
            trace::err(e.what());
        }
    }

    return plid; // platform log id or 0
}

} // namespace dbus

} // namespace util
