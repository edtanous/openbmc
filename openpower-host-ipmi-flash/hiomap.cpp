// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2018 IBM Corp.

#include "config.h"

#include "hiomap.hpp"

#include <endian.h>
#include <host-ipmid/ipmid-api.h>
#include <signal.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <host-ipmid/ipmid-host-cmd-utils.hpp>
#include <host-ipmid/ipmid-host-cmd.hpp>
#include <iostream>
#include <ipmid/api.hpp>
#include <map>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/exception.hpp>
#include <string>
#include <tuple>
#include <utility>

/*

Design and integration notes
============================

The primary motivation of the Host I/O Mapping protocol (HIOMAP) is to mediate
host access to a BMC-controlled flash chip housing the host's boot firmware.

openpower-host-ipmi-flash facilitates the system design of transporting the
HIOMAP protocol[1] over IPMI. This is somewhat abusive of IPMI, basically
treating the BT interface as a mailbox with an interrupt each way between the
BMC and the host.

[1] https://github.com/openbmc/mboxbridge/blob/master/Documentation/protocol.md

Using IPMI in this way has a number of challenges, a lot of them on the host
side where we need to bring up the LPC and BT interfaces to enable IPMI before
accessing the flash, and before any interrupts are enabled. There are also
challenges on the BMC side with the design of the current implementation. We
will cover those here.

BMC-side System Design and Integration Issues
---------------------------------------------

The current design is that we have the HIOMAP daemon, mboxd (to be renamed),
exposing a set of DBus interfaces. Whilst the spec defines the IPMI transport
message packing, mboxd knows nothing of IPMI itself, instead relying on the
DBus interface to receive messages from ipmid. ipmid in-turn knows nothing of
the interfaces communicating with it, also relying on DBus to receive messages
from interface-specific daemons, e.g. btbridged[2].

[2] https://github.com/openbmc/btbridge

For this design to function correctly we must ensure that the daemons are
started and shut down in a reasonable order, however defining that order is
somewhat tricky:

1. systemd uses Wants=/Before=/After= relationships in units to define both
   start-up *and* shutdown order, in stack push / pop order respectively.
2. Clearly ipmid depends on btbridged to receive messages sent by signals and
   replied to by method calls, so it needs a Wants=/After= relationship on
   btbridged
3. mboxd depends on ipmid to receive messages sent by method call, and issues a
   PropertiesChanged signal to notify of state changes.

Point 3. suggests mboxd should have a Wants=/Before= relationship with ipmid to
ensure ipmid can call into mboxd as messages arrive. However, this causes some
grief with shutdown of the BMC, as mboxd needs to issue a state-change
notification when it is shut down to inform the host that will not repsond to
future requests and that the protocol state has been reset. If mboxd has a
Wants=/Before= relationship with ipmid this message will never propagate to the
host, as ipmid will be shut by systemd before mboxd.

The above leads to mboxd having a Wants=/After= relationship with ipmid. This
ensures that if mboxd is restarted on its own the correct state changes will be
propagated to the host. The case where ipmid attempts to call into mboxd's DBus
interface before mboxd is ready is mitigated by the ready bit in the protocol's
BMC status, which will not yet be set, preventing a conforming host from
attempting to contact mboxd.

While this ordering prevents mboxd from being terminated before ipmid, there is
no control over the *scheduling* of processes to ensure the PropertiesChanged
signal emitted by mboxd before mboxd is terminated is seen by ipmid before
*ipmid* is also terminated. This leads to our first implementation wart:

  On the basis that mboxd has a Wants=/After= relationship with ipmid,
  openpower-host-ipmi-flash will emit an HIOMAP BMC status event to the host
  with the value BMC_EVENT_PROTOCOL_RESET upon receiving SIGTERM iff the BMC
  state is not already set to BMC_EVENT_PROTOCOL_RESET.

If ipmid has received SIGTERM the assumption is that it is systemd that is
sending it, and that the Wants=/After= relationship requires that mboxd has
been terminated before ipmid receives SIGTERM. By ensuring
openpower-host-ipmi-flash emits the BMC event state we close the race where the
host is not informed of the termination of mboxd due to scheduling ipmid (to
deliver SIGTERM) prior to scheduling dbus-daemon, where the PropertiesChanged
event would be delivered from mboxd to ipmid.

Observations on the IPMI Specification and Design Details of ipmid
------------------------------------------------------------------

In addition to the system-level design problems with delivering
PropertiesChanged signals during shutdown, IPMI specification and ipmid design
issues exist that make it tedious to ensure that events will be correctly
delivered to the host.

The first necessary observation is that the mechanism for delivering BMC state
change events from mboxd to the host over IPMI uses the SMS ATN bit to indicate
a message is ready for delivery from the BMC to the host system. Retrieving the
BMC state data involves the host recognising that the SMS ATN bit is set,
performing Get Message Flags transaction with the BMC followed by a subsequent
Get Message transaction. Thus, delivery of the HIOMAP protocol's BMC status is
not an atomic event.

The second necessary observation is that the kernel delivers signals
asynchronously. This couples badly with IPMI's event delivery not being atomic:
ipmid can win the race against SIGTERM to receive the PropertiesChanged event
from mboxd, but lose the race to complete delivery to the host.

  On this basis, we need to block the delivery of SIGTERM to ipmid until ipmid
  has completed the set of `SMS ATN`/`Get Message Flags`/`Get Message`
  transactions with the host

One approach to this would be to configure a custom SIGTERM handler that sets
some global application state to indicate that SIGTERM has been delivered. A
better approach that avoids the need for global application state is to simply
block the signal until we are ready to handle it, which we can do via
sigprocmask(2).

The existing design of ipmid makes it feasible to block and unblock
asynchronous SIGTERM as we require. ipmid_send_cmd_to_host() takes a CallBack
function as an argument, which is invoked by
phosphor::host::command::Manager::getNextCommand(). The documentation for
phosphor::host::command::Manager::getNextCommand() says:

  @brief  Extracts the next entry in the queue and returns
          Command and data part of it.

  @detail Also calls into the registered handlers so that they can now
          send the CommandComplete signal since the interface contract
          is that we emit this signal once the message has been
          passed to the host (which is required when calling this)

          Also, if the queue has more commands, then it will alert the
          host

However, its description is not entirely accurate. The callback function is
invoked when ipmid *dequeues* the data to send to the host: Delivery of the
data to the host occurs at some *after* the callback has been invoked.

Invoking the callback before completion of delivery of the data to the host
nullifies the approach of unblocking asynchronous SIGTERM in the callback
associated with sending the HIOMAP BMC state event to the host, as the BMC
kernel can asynchronously terminate the process between the callback being
invoked and the host receiving the BMC state event data.

Overcoming this issue hinges on a significant implementation detail of ipmid:

  ipmid uses an sd_event loop in the main function to pump DBus events.

This leads to a third necessary observation:

  sd_event can be used to process UNIX signals as well as other events by way
  of Linux's signalfd(2) interface.

The fact that sd_event is used to pump DBus events means that ipmid can remain
a single-threaded process. By remaining single-threaded we know that events
processing is sequencial and no two events can be processed simultaneously. A
corollary of this is that DBus events and UNIX signals are serialised with
respect to eachother.

The fourth necessary observation is that we do not need to pump sd_event in
order to complete DBus method calls; sd_bus will handle the pumping independent
of the main loop in order to complete the method invocation.

Implementing Reliable HIOMAP BMC Status Event Delivery
------------------------------------------------------

We achieve reliable delivery of HIOMAP BMC status events in the following way:

1. During plugin initialisation, mask SIGTERM using sigprocmask(2)
2. Subsequent to masking SIGTERM, register
   openpower::flash::hiomap_protocol_reset() as the SIGTERM handler using
   sd_event_add_signal() to hook a signalfd(2) into sd_event
3. openpower::flash::hiomap_protocol_reset() implements the logic to send the
   BMC_EVENT_PROTOCOL_RESET state to the host if necessary, otherwise terminate
   the sd_event loop.
4. If it is necessary to send BMC_EVENT_PROTOCOL_RESET to the host in 3, assign
   a callback handler that terminates the sd_event loop, which is only
   processed after the current iteration is complete.

This process and its use of signalfd integration in the sd_event loop
eliminates the following three races:

1. The scheduler race between mboxd, dbus-daemon and ipmid, by having
   openpower-host-ipmi-flash conditionally deliver the protocol reset event if
   no such message has been received from mboxd
2. The race between delivering the BMC status event to the host and ipmid
   receiving asynchronous SIGTERM after receiving the PropertiesChanged event
   from mboxd
3. The race to deliver the BMC status data to the host after unblocking
   asynchronous SIGTERM in the host command callback and before receiving
   asynchronous SIGTERM.

Ultimately, ipmid could benefit from a redesign that fires the callback *after*
delivering the associated data to the host, but brief inspection determined
that this involved a non-trivial amount of effort.

*/

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

/* XXX: ipmid is currently single-threaded, pumping dbus events in sequence
 * via the main event loop. Thus the code is not forced to be re-entrant. We
 * also know that the callback and DBus event handling will not be running
 * concurrently.
 *
 * ipmid_send_cmd_to_host() takes a callback that doesn't define a context
 * pointer, so instead use a global. active_event_updates gates manipulation of
 * process state, so its definition as a global at least aligns with its use.
 */
static int active_event_updates;

struct hiomap
{
    bus::bus* bus;

    /* Signals */
    bus::match::match* properties;

    /* Protocol state */
    std::map<std::string, int> event_lookup;
    uint8_t bmc_events;
    uint8_t seq;
};

SignalResponse sigtermResponse = SignalResponse::continueExecution;

/* TODO: Replace get/put with packed structs and direct assignment */
template <typename T>
static inline T get(void* buf)
{
    T t;
    std::memcpy(&t, buf, sizeof(t));
    return t;
}

template <typename T>
static inline void put(void* buf, T&& t)
{
    std::memcpy(buf, &t, sizeof(t));
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
    const errno_cc_entry* entry = &errno_cc_map[0];

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

    assert(active_event_updates);
    active_event_updates--;
    if (!active_event_updates)
    {
        sigtermResponse = SignalResponse::continueExecution;
        log<level::DEBUG>("Unblocked SIGTERM");
    }
}

static int hiomap_handle_property_update(struct hiomap* ctx,
                                         sdbusplus::message::message& msg)
{
    using namespace phosphor::logging;

    std::map<std::string, std::variant<bool>> msgData;

    sigtermResponse = SignalResponse::breakExecution;
    if (!active_event_updates)
    {
        sigtermResponse = SignalResponse::breakExecution;
        log<level::DEBUG>("Blocked SIGTERM");
    }
    active_event_updates++;

    std::string iface;
    msg.read(iface, msgData);

    for (auto const& x : msgData)
    {
        if (!ctx->event_lookup.count(x.first))
        {
            /* Unsupported event? */
            continue;
        }

        uint8_t mask = ctx->event_lookup[x.first];
        auto value = std::get<bool>(x.second);

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

static int hiomap_protocol_reset_response(IpmiCmdData cmd, bool status)
{
    // If this is running in signal context, ipmid will shutdown
    // the event queue as the last signal handler
    sigtermResponse = SignalResponse::continueExecution;
    return 0;
}

static int hiomap_protocol_reset(struct hiomap* ctx)
{
    if (ctx->bmc_events == BMC_EVENT_PROTOCOL_RESET)
    {
        // If this is running in signal context, ipmid will shutdown
        // the event queue as the last signal handler
        sigtermResponse = SignalResponse::continueExecution;
        return 0;
    }

    /*
     * Send an attention indicating the hiomapd has died
     * (BMC_EVENT_DAEMON_READY cleared) and that the protocol has been reset
     * (BMC_EVENT_PROTOCOL_RESET set) to indicate to the host that it needs to
     * wait for the BMC to come back and renegotiate the protocol.
     *
     * We know this to be the case in systems that integrate
     * openpower-host-ipmi-flash, as hiomapd's unit depends on
     * phosphor-ipmi-host, and thus hiomapd has been terminated before ipmid
     * receives SIGTERM.
     */
    auto cmd = std::make_pair(IPMI_CMD_HIOMAP_EVENT, BMC_EVENT_PROTOCOL_RESET);

    auto cmdHandler = std::make_tuple(cmd, hiomap_protocol_reset_response);
    ipmid_send_cmd_to_host(cmdHandler);

    return 0;
}

static bus::match::match hiomap_match_properties(struct hiomap* ctx)
{
    auto properties =
        bus::match::rules::propertiesChanged(HIOMAPD_OBJECT, HIOMAPD_IFACE_V2);

    bus::match::match match(
        *ctx->bus, properties,
        std::bind(hiomap_handle_property_update, ctx, std::placeholders::_1));

    return match;
}

static ipmi_ret_t hiomap_reset(ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t data_len, ipmi_context_t context)
{
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE, "Reset");
    try
    {
        ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError& e)
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
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    if (*data_len < 1)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t* reqdata = (uint8_t*)request;
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

        uint8_t* respdata = (uint8_t*)response;

        /* FIXME: Assumes v2! */
        put(&respdata[0], version);
        put(&respdata[1], blockSizeShift);
        put(&respdata[2], htole16(timeout));

        *data_len = 4;
    }
    catch (const exception::SdBusError& e)
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
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "GetFlashInfo");
    try
    {
        auto reply = ctx->bus->call(m);

        uint16_t flashSize, eraseSize;
        reply.read(flashSize, eraseSize);

        uint8_t* respdata = (uint8_t*)response;
        put(&respdata[0], htole16(flashSize));
        put(&respdata[2], htole16(eraseSize));

        *data_len = 4;
    }
    catch (const exception::SdBusError& e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_create_window(struct hiomap* ctx, bool ro,
                                       ipmi_request_t request,
                                       ipmi_response_t response,
                                       ipmi_data_len_t data_len)
{
    if (*data_len < 4)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t* reqdata = (uint8_t*)request;
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

        uint8_t* respdata = (uint8_t*)response;

        /* FIXME: Assumes v2! */
        put(&respdata[0], htole16(lpcAddress));
        put(&respdata[2], htole16(size));
        put(&respdata[4], htole16(offset));

        *data_len = 6;
    }
    catch (const exception::SdBusError& e)
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
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    return hiomap_create_window(ctx, true, request, response, data_len);
}

static ipmi_ret_t hiomap_create_write_window(ipmi_request_t request,
                                             ipmi_response_t response,
                                             ipmi_data_len_t data_len,
                                             ipmi_context_t context)
{
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    return hiomap_create_window(ctx, false, request, response, data_len);
}

static ipmi_ret_t hiomap_close_window(ipmi_request_t request,
                                      ipmi_response_t response,
                                      ipmi_data_len_t data_len,
                                      ipmi_context_t context)
{
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    if (*data_len < 1)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t* reqdata = (uint8_t*)request;
    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "CloseWindow");
    m.append(reqdata[0]);

    try
    {
        auto reply = ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError& e)
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
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    if (*data_len < 4)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t* reqdata = (uint8_t*)request;
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
    catch (const exception::SdBusError& e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_flush(ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t data_len, ipmi_context_t context)
{
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "Flush");

    try
    {
        /* FIXME: No argument call assumes v2 */
        auto reply = ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError& e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_ack(ipmi_request_t request, ipmi_response_t response,
                             ipmi_data_len_t data_len, ipmi_context_t context)
{
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    if (*data_len < 1)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t* reqdata = (uint8_t*)request;
    auto m = ctx->bus->new_method_call(HIOMAPD_SERVICE, HIOMAPD_OBJECT,
                                       HIOMAPD_IFACE_V2, "Ack");
    auto acked = reqdata[0];
    m.append(acked);

    try
    {
        auto reply = ctx->bus->call(m);

        *data_len = 0;
    }
    catch (const exception::SdBusError& e)
    {
        return hiomap_xlate_errno(e.get_errno());
    }

    return IPMI_CC_OK;
}

static ipmi_ret_t hiomap_erase(ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t data_len, ipmi_context_t context)
{
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    if (*data_len < 4)
    {
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t* reqdata = (uint8_t*)request;
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
    catch (const exception::SdBusError& e)
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
    struct hiomap* ctx = static_cast<struct hiomap*>(context);

    if (*data_len < 2)
    {
        *data_len = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    uint8_t* ipmi_req = (uint8_t*)request;
    uint8_t* ipmi_resp = (uint8_t*)response;
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

    uint8_t* flash_req = ipmi_req + 2;
    size_t flash_len = *data_len - 2;
    uint8_t* flash_resp = ipmi_resp + 2;

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
    using namespace phosphor::logging;
    using namespace openpower::flash;

    struct hiomap* ctx = new hiomap();

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

    std::function<SignalResponse(int)> shutdownHandler =
        [ctx](int signalNumber) {
            hiomap_protocol_reset(ctx);
            return sigtermResponse;
        };
    registerSignalHandler(ipmi::prioMax, SIGTERM, shutdownHandler);

    ipmi_register_callback(NETFUN_IBM_OEM, IPMI_CMD_HIOMAP, ctx,
                           openpower::flash::hiomap_dispatch, SYSTEM_INTERFACE);
}
