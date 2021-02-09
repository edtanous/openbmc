#include <attn_logging.hpp>

#include <string>
#include <vector>

namespace attn
{

/**
 * Create a dbus method
 *
 * Find the dbus service associated with the dbus object path and create
 * a dbus method for calling the specified dbus interface and function.
 *
 * @param i_path - dbus object path
 * @param i_interface - dbus method interface
 * @param i_function - dbus interface function
 * @param o_method - method that is created
 * @return non-zero if error
 *
 **/
int dbusMethod(const std::string& i_path, const std::string& i_interface,
               const std::string& i_function,
               sdbusplus::message::message& o_method)
{
    int rc = 1; // assume error

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

            // return the method
            o_method =
                bus.new_method_call(service.c_str(), i_path.c_str(),
                                    i_interface.c_str(), i_function.c_str());

            rc = 0;
        }
        else
        {
            std::string traceMsg =
                "dbusMethod service not found: " + i_path + ", " + i_interface;
            trace<level::INFO>(traceMsg.c_str());
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::string traceMsg = "dbusMethod exception: " + std::string(e.what());
        trace<level::INFO>(traceMsg.c_str());
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

            // parse dbus reply
            response.read(reply);
            plid = std::get<1>(reply); // platform log id is tuple "second"
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            std::string traceMsg =
                "createPel exception: " + std::string(e.what());
            trace<level::INFO>(traceMsg.c_str());
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
        std::stringstream traceMsg;
        traceMsg << filePath.c_str() << " only " << (int)numBytes << " of "
                 << (int)i_buffer.size() << " bytes written";
        auto strobj = traceMsg.str();
        trace<level::ERROR>(strobj.c_str());
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
            std::string traceMsg =
                "createPelRaw exception: " + std::string(e.what());
            trace<level::INFO>(traceMsg.c_str());
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

            // parse dbus reply
            response.read(reply);

            fd = dup(reply); // need to copy (dup) the file descriptor
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            std::string traceMsg = "getPel exception: " + std::string(e.what());
            trace<level::INFO>(traceMsg.c_str());
        }
    }

    return fd; // file descriptor or -1
}

} // namespace attn
