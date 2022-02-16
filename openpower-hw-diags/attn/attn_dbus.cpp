#include <attn/attn_common.hpp>
#include <attn/attn_dbus.hpp>
#include <attn/attn_logging.hpp>
#include <util/dbus.hpp>
#include <util/trace.hpp>

#include <string>
#include <vector>

namespace attn
{

/** @brief Create a dbus method */
int dbusMethod(const std::string& i_path, const std::string& i_interface,
               const std::string& i_function,
               sdbusplus::message::message& o_method)
{
    int rc = RC_DBUS_ERROR; // assume error

    try
    {
        auto bus = sdbusplus::bus::new_system(); // using system dbus

        // we need to find the service implementing the interface
        util::dbus::DBusService service;

        if (0 == util::dbus::findService(i_interface, i_path, service))
        {
            // return the method
            o_method =
                bus.new_method_call(service.c_str(), i_path.c_str(),
                                    i_interface.c_str(), i_function.c_str());

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
