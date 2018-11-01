#include "config.h"

#include "oemhandler.hpp"

#include "elog-errors.hpp"

#include <endian.h>
#include <ipmid/api.h>
#include <stdio.h>
#include <string.h>
#include <systemd/sd-bus.h>

#include <fstream>
#include <functional>
#include <host-interface.hpp>
#include <ipmid-host/cmd.hpp>
#include <memory>
#include <org/open_power/Host/error.hpp>
#include <org/open_power/OCC/Metrics/error.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>

void register_netfn_ibm_oem_commands() __attribute__((constructor));

const char* g_esel_path = "/tmp/esel";
uint16_t g_record_id = 0x0001;
using namespace phosphor::logging;
constexpr auto occMetricsType = 0xDD;

extern const ObjectIDMap invSensors;
const std::map<uint8_t, Entry::Level> severityMap{
    {0x10, Entry::Level::Warning}, // Recoverable error
    {0x20, Entry::Level::Warning}, // Predictive error
    {0x40, Entry::Level::Error},   // Unrecoverable error
    {0x50, Entry::Level::Error},   // Critical error
    {0x60, Entry::Level::Error},   // Error from a diagnostic test
    {0x70, Entry::Level::Warning}, // Recoverable symptom
    {0xFF, Entry::Level::Error},   // Unknown error
};

Entry::Level mapSeverity(const std::string& eSELData)
{
    constexpr size_t severityOffset = 0x4A;

    if (eSELData.size() > severityOffset)
    {
        // Dive in to the IBM log to find the severity
        uint8_t sev = 0xF0 & eSELData[severityOffset];

        auto find = severityMap.find(sev);
        if (find != severityMap.end())
        {
            return find->second;
        }
    }

    // Default to Entry::Level::Error if a matching is not found.
    return Entry::Level::Error;
}

std::string mapCalloutAssociation(const std::string& eSELData)
{
    auto rec = reinterpret_cast<const SELEventRecord*>(&eSELData[0]);
    uint8_t sensor = rec->sensorNum;

    /*
     * Search the sensor number to inventory path mapping to figure out the
     * inventory associated with the ESEL.
     */
    auto found = std::find_if(invSensors.begin(), invSensors.end(),
                              [&sensor](const auto& iter) {
                                  return (iter.second.sensorID == sensor);
                              });
    if (found != invSensors.end())
    {
        return found->first;
    }

    return {};
}

std::string getService(sdbusplus::bus::bus& bus, const std::string& path,
                       const std::string& interface)
{
    auto method = bus.new_method_call(MAPPER_BUS_NAME, MAPPER_OBJ, MAPPER_IFACE,
                                      "GetObject");

    method.append(path);
    method.append(std::vector<std::string>({interface}));

    std::map<std::string, std::vector<std::string>> response;

    try
    {
        auto reply = bus.call(method);

        reply.read(response);
        if (response.empty())
        {
            log<level::ERR>("Error in mapper response for getting service name",
                            entry("PATH=%s", path.c_str()),
                            entry("INTERFACE=%s", interface.c_str()));
            return std::string{};
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ERR>("Error in mapper method call",
                        entry("ERROR=%s", e.what()));
        return std::string{};
    }

    return response.begin()->first;
}

std::string readESEL(const char* fileName)
{
    std::string content{};

    std::ifstream handle(fileName);

    if (handle.fail())
    {
        log<level::ERR>("Failed to open eSEL", entry("FILENAME=%s", fileName));
        return content;
    }

    handle.seekg(0, std::ios::end);
    content.resize(handle.tellg());
    handle.seekg(0, std::ios::beg);
    handle.read(&content[0], content.size());
    handle.close();

    return content;
}

void createOCCLogEntry(const std::string& eSELData)
{
    // Each byte in eSEL is formatted as %02x with a space between bytes and
    // insert '/0' at the end of the character array.
    constexpr auto byteSeperator = 3;

    std::unique_ptr<char[]> data(
        new char[(eSELData.size() * byteSeperator) + 1]());

    for (size_t i = 0; i < eSELData.size(); i++)
    {
        sprintf(&data[i * byteSeperator], "%02x ", eSELData[i]);
    }
    data[eSELData.size() * byteSeperator] = '\0';

    using error = sdbusplus::org::open_power::OCC::Metrics::Error::Event;
    using metadata = org::open_power::OCC::Metrics::Event;

    report<error>(metadata::ESEL(data.get()));
}

void createHostEntry(const std::string& eSELData)
{
    // Each byte in eSEL is formatted as %02x with a space between bytes and
    // insert '/0' at the end of the character array.
    constexpr auto byteSeperator = 3;

    auto sev = mapSeverity(eSELData);
    auto inventoryPath = mapCalloutAssociation(eSELData);

    if (!inventoryPath.empty())
    {
        std::unique_ptr<char[]> data(
            new char[(eSELData.size() * byteSeperator) + 1]());

        for (size_t i = 0; i < eSELData.size(); i++)
        {
            sprintf(&data[i * byteSeperator], "%02x ", eSELData[i]);
        }
        data[eSELData.size() * byteSeperator] = '\0';

        using hosterror = sdbusplus::org::open_power::Host::Error::Event;
        using hostmetadata = org::open_power::Host::Event;

        report<hosterror>(
            sev, hostmetadata::ESEL(data.get()),
            hostmetadata::CALLOUT_INVENTORY_PATH(inventoryPath.c_str()));
    }
}

///////////////////////////////////////////////////////////////////////////////
// For the First partial add eSEL the SEL Record ID and offset
// value should be 0x0000. The extended data needs to be in
// the form of an IPMI SEL Event Record, with Event sensor type
// of 0xDF and Event Message format of 0x04. The returned
// Record ID should be used for all partial eSEL adds.
//
// This function creates a /tmp/esel file to store the
// incoming partial esel.  It is the role of some other
// function to commit the error log in to long term
// storage.  Likely via the ipmi add_sel command.
///////////////////////////////////////////////////////////////////////////////
ipmi_ret_t ipmi_ibm_oem_partial_esel(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                     ipmi_request_t request,
                                     ipmi_response_t response,
                                     ipmi_data_len_t data_len,
                                     ipmi_context_t context)
{
    uint8_t* reqptr = (uint8_t*)request;
    esel_request_t esel_req;
    FILE* fp;
    int r = 0;
    uint8_t rlen;
    ipmi_ret_t rc = IPMI_CC_OK;
    const char* pio;

    esel_req.resid = le16toh((((uint16_t)reqptr[1]) << 8) + reqptr[0]);
    esel_req.selrecord = le16toh((((uint16_t)reqptr[3]) << 8) + reqptr[2]);
    esel_req.offset = le16toh((((uint16_t)reqptr[5]) << 8) + reqptr[4]);
    esel_req.progress = reqptr[6];

    // According to IPMI spec, Reservation ID must be checked.
    if (!checkSELReservation(esel_req.resid))
    {
        // 0xc5 means Reservation Cancelled or Invalid Reservation ID.
        printf("Used Reservation ID = %d\n", esel_req.resid);
        rc = IPMI_CC_INVALID_RESERVATION_ID;

        // clean g_esel_path.
        r = remove(g_esel_path);
        if (r < 0)
            fprintf(stderr, "Error deleting %s\n", g_esel_path);

        return rc;
    }

    // OpenPOWER Host Interface spec says if RecordID and Offset are
    // 0 then then this is a new request
    if (!esel_req.selrecord && !esel_req.offset)
        pio = "wb";
    else
        pio = "rb+";

    rlen = (*data_len) - (uint8_t)(sizeof(esel_request_t));

    if ((fp = fopen(g_esel_path, pio)) != NULL)
    {
        fseek(fp, esel_req.offset, SEEK_SET);
        fwrite(reqptr + (uint8_t)(sizeof(esel_request_t)), rlen, 1, fp);
        fclose(fp);

        *data_len = sizeof(g_record_id);
        memcpy(response, &g_record_id, *data_len);
    }
    else
    {
        fprintf(stderr, "Error trying to perform %s for esel%s\n", pio,
                g_esel_path);
        rc = IPMI_CC_INVALID;
        *data_len = 0;
    }

    // The first bit presents that this is the last partial packet
    // coming down.  If that is the case advance the record id so we
    // don't overlap logs.  This allows anyone to establish a log
    // directory system.
    if (esel_req.progress & 1)
    {
        g_record_id++;

        auto eSELData = readESEL(g_esel_path);

        if (eSELData.empty())
        {
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        // If the eSEL record type is OCC metrics, then create the OCC log
        // entry.
        if (eSELData[2] == occMetricsType)
        {
            createOCCLogEntry(eSELData);
        }
        else
        {
            createHostEntry(eSELData);
        }
    }

    return rc;
}

// Prepare for FW Update.
// Execute needed commands to prepare the system for a fw update from the host.
ipmi_ret_t ipmi_ibm_oem_prep_fw_update(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                       ipmi_request_t request,
                                       ipmi_response_t response,
                                       ipmi_data_len_t data_len,
                                       ipmi_context_t context)
{
    ipmi_ret_t ipmi_rc = IPMI_CC_OK;
    *data_len = 0;

    int rc = 0;
    std::ofstream rwfs_file;
    const char* busname = "org.openbmc.control.Bmc";
    const char* objname = "/org/openbmc/control/bmc0";
    const char* iface = "org.openbmc.control.Bmc";
    sd_bus* bus = ipmid_get_sd_bus_connection();
    sd_bus_message* reply = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r = 0;

    // Set one time flag
    rc = system(
        "fw_setenv openbmconce copy-files-to-ram copy-base-filesystem-to-ram");
    rc = WEXITSTATUS(rc);
    if (rc != 0)
    {
        fprintf(stderr, "fw_setenv openbmconce failed with rc=%d\n", rc);
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    // Touch the image-rwfs file to perform an empty update to force the save
    // in case we're already in ram and the flash is the same causing the ram
    // files to not be copied back to flash
    rwfs_file.open("/run/initramfs/image-rwfs",
                   std::ofstream::out | std::ofstream::app);
    rwfs_file.close();

    // Reboot the BMC for settings to take effect
    r = sd_bus_call_method(bus, busname, objname, iface, "warmReset", &error,
                           &reply, NULL);
    if (r < 0)
    {
        fprintf(stderr, "Failed to reset BMC: %s\n", strerror(-r));
        return -1;
    }
    printf("Warning: BMC is going down for reboot!\n");
    sd_bus_error_free(&error);
    reply = sd_bus_message_unref(reply);

    return ipmi_rc;
}

ipmi_ret_t ipmi_ibm_oem_bmc_factory_reset(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                          ipmi_request_t request,
                                          ipmi_response_t response,
                                          ipmi_data_len_t data_len,
                                          ipmi_context_t context)
{
    sdbusplus::bus::bus bus{ipmid_get_sd_bus_connection()};
    // The timeout for IPMI commands is 5s
    constexpr auto powerOffWait = std::chrono::seconds(1);
    constexpr auto setFactoryWait = std::chrono::seconds(3);

    // Power Off Chassis
    auto service = getService(bus, stateChassisPath, stateChassisIntf);
    if (service.empty())
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }
    sdbusplus::message::variant<std::string> off =
        "xyz.openbmc_project.State.Chassis.Transition.Off";
    auto method = bus.new_method_call(service.c_str(), stateChassisPath,
                                      propertiesIntf, "Set");
    method.append(stateChassisIntf, "RequestedPowerTransition", off);
    try
    {
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ERR>("Error powering off the chassis",
                        entry("ERROR=%s", e.what()));
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    // Wait a few seconds for the chassis to power off
    std::this_thread::sleep_for(powerOffWait);

    // Set Factory Reset
    method = bus.new_method_call(bmcUpdaterServiceName, softwarePath,
                                 factoryResetIntf, "Reset");
    try
    {
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ERR>("Error setting factory reset",
                        entry("ERROR=%s", e.what()));
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    // Wait a few seconds for service that sets the reset env variable to
    // complete before the BMC is rebooted
    std::this_thread::sleep_for(setFactoryWait);

    // Reboot BMC
    service = getService(bus, stateBmcPath, stateBmcIntf);
    if (service.empty())
    {
        log<level::ALERT>("Error getting the service name to reboot the BMC. "
                          "The BMC needs to be manually rebooted to complete "
                          "the factory reset.");
        return IPMI_CC_UNSPECIFIED_ERROR;
    }
    sdbusplus::message::variant<std::string> reboot =
        "xyz.openbmc_project.State.BMC.Transition.Reboot";
    method = bus.new_method_call(service.c_str(), stateBmcPath, propertiesIntf,
                                 "Set");
    method.append(stateBmcIntf, "RequestedBMCTransition", reboot);
    try
    {
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        log<level::ALERT>("Error calling to reboot the BMC. The BMC needs to "
                          "be manually rebooted to complete the factory reset.",
                          entry("ERROR=%s", e.what()));
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    return IPMI_CC_OK;
}

namespace
{
// Storage to keep the object alive during process life
std::unique_ptr<open_power::host::command::Host> opHost
    __attribute__((init_priority(101)));
std::unique_ptr<sdbusplus::server::manager::manager> objManager
    __attribute__((init_priority(101)));
} // namespace

void register_netfn_ibm_oem_commands()
{
    printf("Registering NetFn:[0x%X], Cmd:[0x%X]\n", NETFUN_IBM_OEM,
           IPMI_CMD_PESEL);
    ipmi_register_callback(NETFUN_IBM_OEM, IPMI_CMD_PESEL, NULL,
                           ipmi_ibm_oem_partial_esel, SYSTEM_INTERFACE);

    printf("Registering NetFn:[0x%X], Cmd:[0x%X]\n", NETFUN_OEM,
           IPMI_CMD_PREP_FW_UPDATE);
    ipmi_register_callback(NETFUN_OEM, IPMI_CMD_PREP_FW_UPDATE, NULL,
                           ipmi_ibm_oem_prep_fw_update, SYSTEM_INTERFACE);

    ipmi_register_callback(NETFUN_IBM_OEM, IPMI_CMD_BMC_FACTORY_RESET, NULL,
                           ipmi_ibm_oem_bmc_factory_reset, SYSTEM_INTERFACE);

    // Create new object on the bus
    auto objPath = std::string{CONTROL_HOST_OBJ_MGR} + '/' + HOST_NAME + '0';

    // Add sdbusplus ObjectManager.
    auto& sdbusPlusHandler = ipmid_get_sdbus_plus_handler();
    objManager = std::make_unique<sdbusplus::server::manager::manager>(
        *sdbusPlusHandler, CONTROL_HOST_OBJ_MGR);

    opHost = std::make_unique<open_power::host::command::Host>(
        *sdbusPlusHandler, objPath.c_str());

    // Service for this is provided by phosphor layer systemcmdintf
    // and this will be as part of that.
    return;
}
