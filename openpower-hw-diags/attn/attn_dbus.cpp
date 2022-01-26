#include <attn_common.hpp>
#include <attn_dbus.hpp>
#include <attn_logging.hpp>
#include <util/trace.hpp>

#include <string>
#include <vector>

namespace attn
{

/** @brief Create a dbus method */
int dbusMethod(const std::string& i_path, const std::string& i_interface,
               const std::string& i_function,
               sdbusplus::message::message& o_method,
               const std::string& i_extended)
{
    int rc = RC_DBUS_ERROR; // assume error

    try
    {
        constexpr auto serviceFind   = "xyz.openbmc_project.ObjectMapper";
        constexpr auto pathFind      = "/xyz/openbmc_project/object_mapper";
        constexpr auto interfaceFind = "xyz.openbmc_project.ObjectMapper";
        constexpr auto functionFind  = "GetObject";

        auto bus = sdbusplus::bus::new_system(); // using system dbus

        // method to find service from object path and object interface
        auto method = bus.new_method_call(serviceFind, pathFind, interfaceFind,
                                          functionFind);

        // find the service for specified object path and interface
        method.append(i_path.c_str());
        method.append(std::vector<std::string>({i_interface}));
        auto reply = bus.call(method);

        // dbus call results
        std::map<std::string, std::vector<std::string>> responseFindService;
        reply.read(responseFindService);

        // If we successfully found the service associated with the dbus object
        // path and interface then create a method for the specified interface
        // and function.
        if (!responseFindService.empty())
        {
            auto service = responseFindService.begin()->first;

            // Some methods (e.g. get attribute) take an extended parameter
            if (i_extended == "")
            {
                // return the method
                o_method = bus.new_method_call(service.c_str(), i_path.c_str(),
                                               i_interface.c_str(),
                                               i_function.c_str());
            }
            else
            {
                // return extended method
                o_method =
                    bus.new_method_call(service.c_str(), i_path.c_str(),
                                        i_extended.c_str(), i_function.c_str());
            }

            rc = RC_SUCCESS;
        }
        else
        {
            // This trace will be picked up in event log
            trace::inf("dbusMethod service not found");
            trace::inf(i_path.c_str());
            trace::inf(i_interface.c_str());
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        trace::err("dbusMethod exception");
        trace::err(e.what());
    }

    return rc;
}

/** @brief Create a PEL for the specified event type */
uint32_t createPel(const std::string& i_event,
                   std::map<std::string, std::string>& i_additional,
                   const std::vector<util::FFDCTuple>& i_ffdc)
{
    // CreatePELWithFFDCFiles returns plid
    int plid = 0;

    // Need to provide pid when using create or create-with-ffdc methods
    i_additional.emplace("_PID", std::to_string(getpid()));

    // Sdbus call specifics
    constexpr auto interface = "org.open_power.Logging.PEL";
    constexpr auto function  = "CreatePELWithFFDCFiles";

    sdbusplus::message::message method;

    if (0 == dbusMethod(pathLogging, interface, function, method))
    {
        try
        {
            // append additional dbus call paramaters
            method.append(i_event, levelPelError, i_additional, i_ffdc);

            // using system dbus
            auto bus      = sdbusplus::bus::new_system();
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

/** @brief Create a PEL from raw PEL data */
void createPelRaw(const std::vector<uint8_t>& i_buffer)
{
    // Create FFDC file from buffer data
    util::FFDCFile pelFile{util::FFDCFormat::Text};
    auto fd = pelFile.getFileDescriptor();

    auto filePath = pelFile.getPath(); // path to ffdc file

    size_t numBytes = write(fd, i_buffer.data(), i_buffer.size());
    if (i_buffer.size() != numBytes)
    {
        trace::err("%s only %u of %u bytes written", filePath.c_str(), numBytes,
                   i_buffer.size());
    }

    lseek(fd, 0, SEEK_SET);

    // Additional data for log
    std::map<std::string, std::string> additional;
    additional.emplace("RAWPEL", filePath.string());
    additional.emplace("_PID", std::to_string(getpid()));

    // dbus specifics
    constexpr auto interface = "xyz.openbmc_project.Logging.Create";
    constexpr auto function  = "Create";

    sdbusplus::message::message method;

    if (0 == dbusMethod(pathLogging, interface, function, method))
    {
        try
        {
            // append additional dbus call parameters
            method.append(eventPelTerminate, levelPelError, additional);

            // using system dbus, no reply
            auto bus = sdbusplus::bus::new_system();
            bus.call_noreply(method);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            trace::err("createPelRaw exception");
            trace::err(e.what());
        }
    }
}

/** @brief Get file descriptor of exisitng PEL */
int getPel(const uint32_t i_pelId)
{
    // GetPEL returns file descriptor (int)
    int fd = -1;

    // dbus specifics
    constexpr auto interface = "org.open_power.Logging.PEL";
    constexpr auto function  = "GetPEL";

    sdbusplus::message::message method;

    if (0 == dbusMethod(pathLogging, interface, function, method))
    {
        try
        {
            // additional dbus call parameters
            method.append(i_pelId);

            // using system dbus
            auto bus      = sdbusplus::bus::new_system();
            auto response = bus.call(method);

            // reply will be a unix file descriptor
            sdbusplus::message::unix_fd reply;

            // parse dbus response into reply
            response.read(reply);

            fd = dup(reply); // need to copy (dup) the file descriptor
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            trace::err("getPel exception");
            trace::err(e.what());
        }
    }

    return fd; // file descriptor or -1
}

} // namespace attn
