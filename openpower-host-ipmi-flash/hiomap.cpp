// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2018 IBM Corp.

#include "config.h"

#include "hiomap.hpp"

#include <endian.h>
#include <host-ipmid/ipmid-api.h>
#include <string.h>
#include <systemd/sd-bus.h>

#include <fstream>
#include <functional>
#include <host-ipmid/ipmid-host-cmd-utils.hpp>
#include <host-ipmid/ipmid-host-cmd.hpp>
#include <iostream>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/exception.hpp>

using namespace sdbusplus;
using namespace phosphor::host::command;

static void register_openpower_hiomap_commands() __attribute__((constructor));

namespace openpower
{
namespace flash
{
constexpr auto BMC_EVENT_DAEMON_READY = 1 << 7;
constexpr auto BMC_EVENT_FLASH_CTRL_LOST = 1 << 6;
constexpr auto BMC_EVENT_WINDOW_RESET = 1 << 1;
constexpr auto BMC_EVENT_PROTOCOL_RESET = 1 << 0;

constexpr auto IPMI_CMD_HIOMAP_EVENT = 0x0f;

constexpr auto HIOMAPD_SERVICE = "xyz.openbmc_project.Hiomapd";
constexpr auto HIOMAPD_OBJECT = "/xyz/openbmc_project/Hiomapd";
constexpr auto HIOMAPD_IFACE = "xyz.openbmc_project.Hiomapd.Protocol";
constexpr auto HIOMAPD_IFACE_V2 = "xyz.openbmc_project.Hiomapd.Protocol.V2";

constexpr auto DBUS_IFACE_PROPERTIES = "org.freedesktop.DBus.Properties";

struct hiomap
{
    bus::bus *bus;

    /* Signals */
    bus::match::match *properties;
    bus::match::match *window_reset;
    bus::match::match *bmc_reboot;

    /* Protocol state */
    std::map<std::string, int> event_lookup;
    uint8_t bmc_events;
    uint8_t seq;
};

/* TODO: Replace get/put with packed structs and direct assignment */
template <typename T> static inline T get(void *buf)
{
    T t;
    memcpy(&t, buf, sizeof(t));
    return t;
}

template <typename T> static inline void put(void *buf, T &&t)
{
    memcpy(buf, &t, sizeof(t));
}

typedef ipmi_ret_t (*hiomap_command)(ipmi_request_t req, ipmi_response_t resp,
                                     ipmi_data_len_t data_len,
                                     ipmi_context_t context);

struct errno_cc_entry
{
    int err;
    int cc;
};

static const errno_cc_entry errno_cc_map[] = {
    {0, IPMI_CC_OK},
    {EBUSY, IPMI_CC_BUSY},
    {ENOTSUP, IPMI_CC_INVALID},
    {ETIMEDOUT, 0xc3}, /* FIXME: Replace when defined in ipmid-api.h */
    {ENOSPC, 0xc4},    /* FIXME: Replace when defined in ipmid-api.h */
    {EINVAL, IPMI_CC_PARM_OUT_OF_RANGE},
    {ENODEV, IPMI_CC_SENSOR_INVALID},
    {EPERM, IPMI_CC_INSUFFICIENT_PRIVILEGE},
    {EACCES, IPMI_CC_INSUFFICIENT_PRIVILEGE},
    {-1, IPMI_CC_UNSPECIFIED_ERROR},
};

static int hiomap_xlate_errno(int err)
{
    const errno_cc_entry *entry = &errno_cc_map[0];

    while (!(entry->err == err || entry->err == -1))
    {
        entry++;
    }

    return entry->cc;
}

static void ipmi_hiomap_event_response(IpmiCmdData cmd, bool status)
{
    using namespace phosphor::logging;

    if (!status)
    {
        log<level::ERR>("Failed to deliver host command",
                        entry("SEL_COMMAND=%x:%x", cmd.first, cmd.second));
    }
}

static int hiomap_handle_property_update(struct hiomap *ctx,
                                         sdbusplus::message::message &msg)
{
    std::map<std::string, sdbusplus::message::variant<bool>> msgData;

    std::string iface;
    msg.read(iface, msgData);

    for (auto const &x : msgData)
    {
        if (!ctx->event_lookup.count(x.first))
        {
            /* Unsupported event? */
            continue;
        }

        uint8_t mask = ctx->event_lookup[x.first];
        auto value = sdbusplus::message::variant_ns::get<bool>(x.second);

        if (value)
        {
            ctx->bmc_events |= mask;
        }
        else
        {
            ctx->bmc_events &= ~mask;
        }
    }

    auto cmd = std::make_pair(IPMI_CMD_HIOMAP_EVENT, ctx->bmc_events);

    ipmid_send_cmd_to_host(std::make_tuple(cmd, ipmi_hiomap_event_response));

    return 0;
}

static bus::match::match hiomap_match_properties(struct hiomap *ctx)
{
    auto properties =
        bus::match::rules::propertiesChanged(HIOMAPD_OBJECT, HIOMAPD_IFACE_V2);

    bus::match::match match(
        *ctx->bus, properties,
        std::bind(hiomap_handle_property_update, ctx, std::placeholders::_1));

    return match;
}

static int hiomap_handle_signal_v2(struct hiomap *ctx, const char *name)
{
    ctx->bmc_events |= ctx->event_lookup[name];

    auto cmd = std::make_pair(IPMI_CMD_HIOMAP_EVENT, ctx->bmc_events);

    ipmid_send_cmd_to_host(std::make_tuple(cmd, ipmi_hiomap_event_response));

    return 0;
}

static bus::match::match hiomap_match_signal_v2(struct hiomap *ctx,
                                                const char *name)
{
    using namespace bus::match;

    auto signals = rules::type::signal() + rules::path(HIOMAPD_OBJECT) +
                   rules::interface(HIOMAPD_IFACE_V2) + rules::member(name);

    bus::match::match match(*ctx->bus, signals,
                            std::bind(hiomap_handle_signal_v2, ctx, name));

    return match;
}

static ipmi_ret_t hiomap_reset(ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t data_len, ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE, "Reset");
    try
    {
        ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_get_info(ipmi_request_t request,
                                  ipmi_response_t response,
                                  ipmi_data_len_t data_len,
                                  ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    if (*data_len < 1)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t *reqdata = (uint8_t *)request;
    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE, "GetInfo");
    m.append(reqdata[0]);

    try
    {
        auto reply = ctx->bus->call(m);

        uint8_t version;
        uint8_t blockSizeShift;
        uint16_t timeout;
        reply.read(version, blockSizeShift, timeout);

        uint8_t *respdata = (uint8_t *)response;

        /* FIXME: Assumes v2! */
        put(&respdata[0], version);
        put(&respdata[1], blockSizeShift);
        put(&respdata[2], htole16(timeout));

        *data_len = 4;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_get_flash_info(ipmi_request_t request,
                                        ipmi_response_t response,
                                        ipmi_data_len_t data_len,
                                        ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "GetFlashInfo");
    try
    {
        auto reply = ctx->bus->call(m);

        uint16_t flashSize, eraseSize;
        reply.read(flashSize, eraseSize);

        uint8_t *respdata = (uint8_t *)response;
        put(&respdata[0], htole16(flashSize));
        put(&respdata[2], htole16(eraseSize));

        *data_len = 4;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_create_window(struct hiomap *ctx, bool ro,
                                       ipmi_request_t request,
                                       ipmi_response_t response,
                                       ipmi_data_len_t data_len)
{
    if (*data_len < 4)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t *reqdata = (uint8_t *)request;
    auto windowType = ro ? "CreateReadWindow" : "CreateWriteWindow";

    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, windowType);
    m.append(le16toh(get<uint16_t>(&reqdata[0])));
    m.append(le16toh(get<uint16_t>(&reqdata[2])));

    try
    {
        auto reply = ctx->bus->call(m);

        uint16_t lpcAddress, size, offset;
        reply.read(lpcAddress, size, offset);

        uint8_t *respdata = (uint8_t *)response;

        /* FIXME: Assumes v2! */
        put(&respdata[0], htole16(lpcAddress));
        put(&respdata[2], htole16(size));
        put(&respdata[4], htole16(offset));

        *data_len = 6;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_create_read_window(ipmi_request_t request,
                                            ipmi_response_t response,
                                            ipmi_data_len_t data_len,
                                            ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    return hiomap_create_window(ctx, true, request, response, data_len);
}

static ipmi_ret_t hiomap_create_write_window(ipmi_request_t request,
                                             ipmi_response_t response,
                                             ipmi_data_len_t data_len,
                                             ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    return hiomap_create_window(ctx, false, request, response, data_len);
}

static ipmi_ret_t hiomap_close_window(ipmi_request_t request,
                                      ipmi_response_t response,
                                      ipmi_data_len_t data_len,
                                      ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    if (*data_len < 1)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t *reqdata = (uint8_t *)request;
    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "CloseWindow");
    m.append(reqdata[0]);

    try
    {
        auto reply = ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_mark_dirty(ipmi_request_t request,
                                    ipmi_response_t response,
                                    ipmi_data_len_t data_len,
                                    ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    if (*data_len < 4)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t *reqdata = (uint8_t *)request;
    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "MarkDirty");
    /* FIXME: Assumes v2 */
    m.append(le16toh(get<uint16_t>(&reqdata[0]))); /* offset */
    m.append(le16toh(get<uint16_t>(&reqdata[2]))); /* size */

    try
    {
        auto reply = ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_flush(ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t data_len, ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "Flush");

    try
    {
        /* FIXME: No argument call assumes v2 */
        auto reply = ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_ack(ipmi_request_t request, ipmi_response_t response,
                             ipmi_data_len_t data_len, ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    if (*data_len < 1)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t *reqdata = (uint8_t *)request;
    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "Ack");
    auto acked = reqdata[0];
    m.append(acked);

    try
    {
        auto reply = ctx->bus->call(m);

        /* Update our cache: Necessary because the signals do not carry a value
         */
        ctx->bmc_events &= ~acked;

        *data_len = 0;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_erase(ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t data_len, ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    if (*data_len < 4)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t *reqdata = (uint8_t *)request;
    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "Erase");
    /* FIXME: Assumes v2 */
    m.append(le16toh(get<uint16_t>(&reqdata[0]))); /* offset */
    m.append(le16toh(get<uint16_t>(&reqdata[2]))); /* size */

    try
    {
        auto reply = ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError &e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

#define HIOMAP_C_RESET 1
#define HIOMAP_C_GET_INFO 2
#define HIOMAP_C_GET_FLASH_INFO 3
#define HIOMAP_C_CREATE_READ_WINDOW 4
#define HIOMAP_C_CLOSE_WINDOW 5
#define HIOMAP_C_CREATE_WRITE_WINDOW 6
#define HIOMAP_C_MARK_DIRTY 7
#define HIOMAP_C_FLUSH 8
#define HIOMAP_C_ACK 9
#define HIOMAP_C_ERASE 10

static const hiomap_command hiomap_commands[] = {
    [0] = NULL, /* Invalid command ID */
    [HIOMAP_C_RESET] = hiomap_reset,
    [HIOMAP_C_GET_INFO] = hiomap_get_info,
    [HIOMAP_C_GET_FLASH_INFO] = hiomap_get_flash_info,
    [HIOMAP_C_CREATE_READ_WINDOW] = hiomap_create_read_window,
    [HIOMAP_C_CLOSE_WINDOW] = hiomap_close_window,
    [HIOMAP_C_CREATE_WRITE_WINDOW] = hiomap_create_write_window,
    [HIOMAP_C_MARK_DIRTY] = hiomap_mark_dirty,
    [HIOMAP_C_FLUSH] = hiomap_flush,
    [HIOMAP_C_ACK] = hiomap_ack,
    [HIOMAP_C_ERASE] = hiomap_erase,
};

/* FIXME: Define this in the "right" place, wherever that is */
/* FIXME: Double evaluation */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static ipmi_ret_t hiomap_dispatch(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                  ipmi_request_t request,
                                  ipmi_response_t response,
                                  ipmi_data_len_t data_len,
                                  ipmi_context_t context)
{
    struct hiomap *ctx = static_cast<struct hiomap *>(context);

    if (*data_len < 2)
    {
        *data_len = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t *ipmi_req = (uint8_t *)request;
    uint8_t *ipmi_resp = (uint8_t *)response;
    uint8_t hiomap_cmd = ipmi_req[0];

    if (hiomap_cmd == 0 || hiomap_cmd > ARRAY_SIZE(hiomap_commands) - 1)
    {
        *data_len = 0;
        return IPMI_CC_PARM_OUT_OF_RANGE;
    }

    bool is_unversioned =
        (hiomap_cmd == HIOMAP_C_RESET || hiomap_cmd == HIOMAP_C_GET_INFO ||
         hiomap_cmd == HIOMAP_C_ACK);
    if (!is_unversioned && ctx->seq == ipmi_req[1])
    {
        *data_len = 0;
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    ctx->seq = ipmi_req[1];

    uint8_t *flash_req = ipmi_req + 2;
    size_t flash_len = *data_len - 2;
    uint8_t *flash_resp = ipmi_resp + 2;

    ipmi_ret_t cc =
        hiomap_commands[hiomap_cmd](flash_req, flash_resp, &flash_len, context);
    if (cc != IPMI_CC_OK)
    {
        *data_len = 0;
        return cc;
    }

    /* Populate the response command and sequence */
    ipmi_resp[0] = hiomap_cmd;
    ipmi_resp[1] = ctx->seq;

    *data_len = flash_len + 2;

    return cc;
}
} // namespace flash
} // namespace openpower

static void register_openpower_hiomap_commands()
{
    using namespace openpower::flash;

    /* FIXME: Clean this up? Can we unregister? */
    struct hiomap *ctx = new hiomap();

    /* Initialise mapping from signal and property names to status bit */
    ctx->event_lookup["DaemonReady"] = BMC_EVENT_DAEMON_READY;
    ctx->event_lookup["FlashControlLost"] = BMC_EVENT_FLASH_CTRL_LOST;
    ctx->event_lookup["WindowReset"] = BMC_EVENT_WINDOW_RESET;
    ctx->event_lookup["ProtocolReset"] = BMC_EVENT_PROTOCOL_RESET;

    ctx->bus = new bus::bus(ipmid_get_sd_bus_connection());

    /* Initialise signal handling */

    /*
     * Can't use temporaries here because that causes SEGFAULTs due to slot
     * destruction (!?), so enjoy the weird wrapping.
     */
    ctx->properties =
        new bus::match::match(std::move(hiomap_match_properties(ctx)));
    ctx->bmc_reboot = new bus::match::match(
        std::move(hiomap_match_signal_v2(ctx, "ProtocolReset")));
    ctx->window_reset = new bus::match::match(
        std::move(hiomap_match_signal_v2(ctx, "WindowReset")));

    ipmi_register_callback(NETFUN_IBM_OEM, IPMI_CMD_HIOMAP, ctx,
                           openpower::flash::hiomap_dispatch, SYSTEM_INTERFACE);
}
