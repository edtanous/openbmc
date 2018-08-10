// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2018 IBM Corp.

#include "config.h"

#include "hiomap.hpp"

#include <endian.h>
#include <host-ipmid/ipmid-api.h>
#include <string.h>
#include <systemd/sd-bus.h>

#include <fstream>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>

using namespace sdbusplus;

static void register_openpower_hiomap_commands() __attribute__((constructor));

namespace openpower
{
namespace flash
{

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

static ipmi_ret_t hiomap_get_info(ipmi_request_t request,
                                  ipmi_response_t response,
                                  ipmi_data_len_t data_len,
                                  ipmi_context_t context)
{
    if (*data_len < 1)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t *reqdata = (uint8_t *)request;
    auto b = bus::new_system();
    auto m = b.new_method_call(
        "xyz.openbmc_project.Hiomapd", "/xyz/openbmc_project/Hiomapd",
        "xyz.openbmc_project.Hiomapd.Protocol", "GetInfo");
    m.append(reqdata[0]);

    try
    {
        auto reply = b.call(m);

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

static const hiomap_command hiomap_commands[] = {
    [0] = NULL, /* 0 is an invalid command ID */
    [1] = NULL, /* RESET */
    [2] = hiomap_get_info,
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
    put(&ipmi_resp[0], ipmi_req[0]);
    put(&ipmi_resp[1], ipmi_req[1]);

    *data_len = flash_len + 2;

    return cc;
}
} // namespace flash
} // namespace openpower

static void register_openpower_hiomap_commands()
{
    ipmi_register_callback(NETFUN_IBM_OEM, IPMI_CMD_HIOMAP, NULL,
                           openpower::flash::hiomap_dispatch, SYSTEM_INTERFACE);
}
