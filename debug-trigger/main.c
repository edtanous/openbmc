// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 IBM Corp.

/*
 * debug-trigger listens for an external signal that the BMC is in some way unresponsive. When a
 * signal is received it triggers a crash to collect debug data and reboots the system in the hope
 * that it will recover.
 *
 * Usage: debug-trigger [SOURCE] [SINK]
 *
 * Options:
 *  --sink-actions=ACTION
 *	Set the class of sink action(s) to be used. Can take the value of 'sysrq' or 'dbus'.
 *	Defaults to 'sysrq'.
 *
 * Examples:
 *  debug-trigger
 *	Set the source as stdin, the sink as stdout, and use the default 'sysrq' set of sink
 *	actions. Useful for testing.
 *
 *  debug-trigger --sink-actions=sysrq
 *	Explicitly use the 'sysrq' set of sink actions with stdin as the source and stdout as the
 *	sink.
 *
 *  debug-trigger /dev/serio_raw0 /proc/sysrq-trigger
 *	Open /dev/serio_raw0 as the source and /proc/sysrq-trigger as the sink, with the default
 *	'sysrq' set of sink actions. When 'D' is read from /dev/serio_raw0 'c' will be written to
 *	/proc/sysrq-trigger, causing a kernel panic. When 'R' is read from /dev/serio_raw0 'b' will
 *	be written to /proc/sysrq-trigger, causing an immediate reboot of the system.
 *
 *  dbug-trigger --sink-actions=dbus /dev/serio_raw0
 *	Open /dev/serio_raw0 as the source and configure the 'dbus' set of sink actions. When 'D' is
 *	read from /dev/serio_raw0 create a dump via phosphor-debug-collector by calling through its
 *	D-Bus interface, then reboot the system by starting systemd's 'reboot.target'
 */
#define _GNU_SOURCE

#include "config.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <linux/reboot.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

struct sd_bus;

struct debug_source_ops {
	int (*poll)(void *ctx, char *op);
};

struct debug_source {
	const struct debug_source_ops *ops;
	void *ctx;
};

struct debug_source_basic {
	int source;
};

struct debug_source_dbus {
	struct sd_bus *bus;
#define DBUS_SOURCE_PFD_SOURCE	0
#define DBUS_SOURCE_PFD_DBUS	1
	struct pollfd pfds[2];
};

struct debug_sink_ops {
	void (*debug)(void *ctx);
	void (*reboot)(void *ctx);
};

struct debug_sink {
	const struct debug_sink_ops *ops;
	void *ctx;
};

struct debug_sink_sysrq {
	int sink;
};

struct debug_sink_dbus {
	struct sd_bus *bus;
};

static void sysrq_sink_debug(void *ctx)
{
	struct debug_sink_sysrq *sysrq = ctx;
	/* https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/admin-guide/sysrq.rst?h=v5.16#n93 */
	static const char action = 'c';
	ssize_t rc;

	sync();

	if ((rc = write(sysrq->sink, &action, sizeof(action))) == sizeof(action))
		return;

	if (rc == -1) {
		warn("Failed to execute debug command");
	} else {
		warnx("Failed to execute debug command: %zd", rc);
	}
}

static void sysrq_sink_reboot(void *ctx)
{
	struct debug_sink_sysrq *sysrq = ctx;
	/* https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/admin-guide/sysrq.rst?h=v5.16#n90 */
	static const char action = 'b';
	ssize_t rc;

	sync();

	if ((rc = write(sysrq->sink, &action, sizeof(action))) == sizeof(action))
		return;

	if (rc == -1) {
		warn("Failed to reboot BMC");
	} else {
		warnx("Failed to reboot BMC: %zd", rc);
	}
}

static int basic_source_poll(void *ctx, char *op)
{
	struct debug_source_basic *basic = ctx;
	ssize_t ingress;

	if ((ingress = read(basic->source, op, 1)) != 1) {
		if (ingress < 0) {
			warn("Failed to read from basic source");
			return -errno;
		}

		/* Unreachable */
		errx(EXIT_FAILURE, "Bad read, requested 1 got %zd", ingress);
	}

	return 0;
}

const struct debug_sink_ops sysrq_sink_ops = {
	.debug = sysrq_sink_debug,
	.reboot = sysrq_sink_reboot,
};

const struct debug_source_ops basic_source_ops = {
	.poll = basic_source_poll,
};

#if HAVE_SYSTEMD
#include <systemd/sd-bus.h>

static void dbus_sink_reboot(void *ctx);
static int dbus_sink_dump_progress(sd_bus_message *m, void *userdata,
				   sd_bus_error *ret_error __attribute__((unused)))
{
	struct debug_sink_dbus *dbus = userdata;
	const char *status;
	const char *iface;
	int rc;

	// sa{sv}as
	rc = sd_bus_message_read_basic(m, 's', &iface);
	if (rc < 0) {
		warnx("Failed to extract interface from PropertiesChanged signal: %s",
		      strerror(-rc));
		return rc;
	}

	/* Bail if it's not an update to the Progress interface */
	if (strcmp(iface, "xyz.openbmc_project.Common.Progress"))
		return 0;

	rc = sd_bus_message_enter_container(m, 'a', "{sv}");
	if (rc < 0)
		return rc;

	if (!rc)
		return 0;

	status = NULL;
	while (1) {
		const char *member;

		rc = sd_bus_message_enter_container(m, 'e', "sv");
		if (rc < 0)
			return rc;

		if (!rc)
			break;

		rc = sd_bus_message_read_basic(m, 's', &member);
		if (rc < 0) {
			warnx("Failed to extract member name from PropertiesChanged signal: %s",
			      strerror(-rc));
			return rc;
		}

		if (!strcmp(member, "Status")) {
			rc = sd_bus_message_enter_container(m, 'v', "s");
			if (rc < 0) {
				warnx("Failed to enter variant container in PropertiesChanged signal: %s",
				      strerror(-rc));
				return rc;
			}

			if (!rc)
				goto exit_dict_container;

			rc = sd_bus_message_read_basic(m, 's', &status);
			if (rc < 0) {
				warnx("Failed to extract status value from PropertiesChanged signal: %s",
				      strerror(-rc));
				return rc;
			}

			sd_bus_message_exit_container(m);
		} else {
			rc = sd_bus_message_skip(m, "v");
			if (rc < 0) {
				warnx("Failed to skip variant for unrecognised member %s in PropertiesChanged signal: %s",
				      member, strerror(-rc));
				return rc;
			}
		}

exit_dict_container:
		sd_bus_message_exit_container(m);
	}

	sd_bus_message_exit_container(m);

	if (!status)
		return 0;

	printf("Dump progress on %s: %s\n", sd_bus_message_get_path(m), status);

	/* If we're finished with the dump, reboot the system */
	if (!strcmp(status, "xyz.openbmc_project.Common.Progress.OperationStatus.Completed")) {
		sd_bus_slot *slot = sd_bus_get_current_slot(dbus->bus);
		sd_bus_slot_unref(slot);
		dbus_sink_reboot(userdata);
	}

	return 0;
}

static void dbus_sink_debug(void *ctx)
{
	sd_bus_error ret_error = SD_BUS_ERROR_NULL;
	struct debug_sink_dbus *dbus = ctx;
	sd_bus_message *reply;
	sd_bus_slot *slot;
	const char *path;
	char *status;
	int rc;

	/* Start a BMC dump */
	rc = sd_bus_call_method(dbus->bus,
				"xyz.openbmc_project.Dump.Manager",
				"/xyz/openbmc_project/dump/bmc",
				"xyz.openbmc_project.Dump.Create",
				"CreateDump",
				&ret_error,
				&reply, "a{sv}", 0);
	if (rc < 0) {
		warnx("Failed to call CreateDump: %s", strerror(-rc));
		return;
	}

	/* Extract the dump path */
	rc = sd_bus_message_read_basic(reply, 'o', &path);
	if (rc < 0) {
		warnx("Failed to extract dump object path: %s", strerror(-rc));
		goto cleanup_reply;
	}

	/* Set up a match watching for completion of the dump */
	rc = sd_bus_match_signal(dbus->bus,
				 &slot,
				 "xyz.openbmc_project.Dump.Manager",
				 path,
				 "org.freedesktop.DBus.Properties",
				 "PropertiesChanged",
				 dbus_sink_dump_progress,
				 ctx);
	if (rc < 0) {
		warnx("Failed to add signal match for progress status on dump object %s: %s",
		      path, strerror(-rc));
		goto cleanup_reply;
	}

	/*
	 * Mark the slot as 'floating'. If a slot is _not_ marked as floating it holds a reference
	 * to the bus, and the bus will stay alive so long as the slot is referenced. If the slot is
	 * instead marked floating the relationship is inverted: The lifetime of the slot is defined
	 * in terms of the bus, which means we relieve ourselves of having to track the lifetime of
	 * the slot.
	 *
	 * For more details see `man 3 sd_bus_slot_set_floating`, also documented here:
	 *
	 * https://www.freedesktop.org/software/systemd/man/sd_bus_slot_set_floating.html
	 */
	rc = sd_bus_slot_set_floating(slot, 0);
	if (rc < 0) {
		warnx("Failed to mark progress match slot on %s as floating: %s",
		      path, strerror(-rc));
		goto cleanup_reply;
	}

	printf("Registered progress match on dump object %s\n", path);

	/* Now that the match is set up, check the current value in case we missed any updates */
	rc = sd_bus_get_property_string(dbus->bus,
					"xyz.openbmc_project.Dump.Manager",
					path,
					"xyz.openbmc_project.Common.Progress",
					"Status",
					&ret_error,
					&status);
	if (rc < 0) {
		warnx("Failed to get progress status property on dump object %s: %s",
		      path, strerror(-rc));
		sd_bus_slot_unref(slot);
		goto cleanup_reply;
	}

	printf("Dump state for %s is currently %s\n", path, status);

	/*
	 * If we're finished with the dump, reboot the system. If the dump isn't finished the reboot
	 * will instead take place via the dbus_sink_dump_progress() callback on the match.
	 */
	if (!strcmp(status, "xyz.openbmc_project.Common.Progress.OperationStatus.Completed")) {
		sd_bus_slot_unref(slot);
		dbus_sink_reboot(ctx);
	}

cleanup_reply:
	sd_bus_message_unref(reply);
}

static void dbus_sink_reboot(void *ctx)
{
	sd_bus_error ret_error = SD_BUS_ERROR_NULL;
	struct debug_sink_dbus *dbus = ctx;
	sd_bus_message *reply;
	int rc;

	warnx("Rebooting the system");

	rc = sd_bus_call_method(dbus->bus,
				"org.freedesktop.systemd1",
				"/org/freedesktop/systemd1",
				"org.freedesktop.systemd1.Manager",
				"StartUnit",
				&ret_error,
				&reply,
				"ss",
				"reboot.target",
				"replace-irreversibly");
	if (rc < 0) {
		warnx("Failed to start reboot.target: %s", strerror(-rc));
	}
}

static int dbus_source_poll(void *ctx, char *op)
{
	struct debug_source_dbus *dbus = ctx;
	int rc;

	while (1) {
		struct timespec tsto, *ptsto;
		uint64_t dbusto;

		/* See SD_BUS_GET_FD(3) */
		dbus->pfds[DBUS_SOURCE_PFD_DBUS].fd = sd_bus_get_fd(dbus->bus);
		dbus->pfds[DBUS_SOURCE_PFD_DBUS].events = sd_bus_get_events(dbus->bus);
		rc = sd_bus_get_timeout(dbus->bus, &dbusto);
		if (rc < 0)
			return rc;

		if (dbusto == UINT64_MAX) {
			ptsto = NULL;
		} else if (dbus->pfds[DBUS_SOURCE_PFD_DBUS].events == 0) {
			ptsto = NULL;
		} else {
#define MSEC_PER_SEC 1000U
#define USEC_PER_SEC (MSEC_PER_SEC * 1000U)
#define NSEC_PER_SEC (USEC_PER_SEC * 1000U)
#define NSEC_PER_USEC (NSEC_PER_SEC / USEC_PER_SEC)
			tsto.tv_sec = dbusto / USEC_PER_SEC;
			tsto.tv_nsec = (dbusto % USEC_PER_SEC) * NSEC_PER_USEC;
			ptsto = &tsto;
		}

		if ((rc = ppoll(dbus->pfds, ARRAY_SIZE(dbus->pfds), ptsto, NULL)) < 0) {
			warn("Failed polling source fds");
			return -errno;
		}

		if (dbus->pfds[DBUS_SOURCE_PFD_SOURCE].revents) {
			ssize_t ingress;

			if ((ingress = read(dbus->pfds[DBUS_SOURCE_PFD_SOURCE].fd, op, 1)) != 1) {
				if (ingress < 0) {
					warn("Failed to read from basic source");
					return -errno;
				}

				errx(EXIT_FAILURE, "Bad read, requested 1 got %zd", ingress);
			}

			return 0;
		}

		if (dbus->pfds[DBUS_SOURCE_PFD_DBUS].revents) {
			if ((rc = sd_bus_process(dbus->bus, NULL)) < 0) {
				warnx("Failed processing inbound D-Bus messages: %s",
				      strerror(-rc));
				return rc;
			}
		}
	}
}
#else
static void dbus_sink_debug(void *ctx)
{
	warnx("%s: Configured without systemd, dbus sinks disabled", __func__);
}

static void dbus_sink_reboot(void *ctx)
{
	warnx("%s: Configured without systemd, dbus sinks disabled", __func__);
}

static int dbus_source_poll(void *ctx, char *op)
{
	errx(EXIT_FAILURE, "Configured without systemd, dbus sources disabled", __func__);
}
#endif

const struct debug_sink_ops dbus_sink_ops = {
	.debug = dbus_sink_debug,
	.reboot = dbus_sink_reboot,
};

const struct debug_source_ops dbus_source_ops = {
	.poll = dbus_source_poll,
};

static int process(struct debug_source *source, struct debug_sink *sink)
{
	char command;
	int rc;

	while (!(rc = source->ops->poll(source->ctx, &command))) {
		switch (command) {
		case 'D':
			sink->ops->debug(sink->ctx);
			break;
		case 'R':
			sink->ops->reboot(sink->ctx);
			break;
		default:
			warnx("Unexpected command: 0x%02x (%c)", command, command);
		}
	}

	if (rc < 0)
		warnx("Failed to poll source: %s", strerror(-rc));

	return rc;
}

int main(int argc, char * const argv[])
{
	struct debug_source_basic basic_source;
	struct debug_source_dbus dbus_source;
	struct debug_sink_sysrq sysrq_sink;
	struct debug_sink_dbus dbus_sink;
	const char *sink_actions = NULL;
	struct debug_source source;
	struct debug_sink sink;
	char devnode[PATH_MAX];
	char *devid;
	int sourcefd;
	int sinkfd;

	/* Option processing */
	while (1) {
		static struct option long_options[] = {
			{"sink-actions", required_argument, 0, 's'},
			{0, 0, 0, 0},
		};
		int c;

		c = getopt_long(argc, argv, "", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 's':
			sink_actions = optarg;
			break;
		default:
			break;
		}
	}

	/*
	 * The default behaviour sets the source file descriptor as stdin and the sink file
	 * descriptor as stdout. This allows trivial testing on the command-line with just a
	 * keyboard and without crashing the system.
	 */
	sourcefd = 0;
	sinkfd = 1;

	/* Handle the source path argument, if any */
	if (optind < argc) {
		char devpath[PATH_MAX];

		/*
		 * To make our lives easy with udev we take the basename of the source argument and
		 * look for it in /dev. This allows us to use %p (the devpath specifier) in the udev
		 * rule to pass the device of interest to the systemd unit.
		 */
		strncpy(devpath, argv[optind], sizeof(devpath));
		devpath[PATH_MAX - 1] = '\0';
		devid = basename(devpath);

		strncpy(devnode, "/dev/", sizeof(devnode));
		strncat(devnode, devid, sizeof(devnode));
		devnode[PATH_MAX - 1] = '\0';

		if ((sourcefd = open(devnode, O_RDONLY)) == -1)
			err(EXIT_FAILURE, "Failed to open %s", devnode);

		optind++;
	}

	/*
	 * Handle the sink path argument, if any. If sink_actions hasn't been set via the
	 * --sink-actions option, then default to 'sysrq'. Otherwise, if --sink-actions=sysrq has
	 * been passed, do as we're told and use the 'sysrq' sink actions.
	 */
	if (!sink_actions || !strcmp("sysrq", sink_actions)) {
		if (optind < argc) {
			/*
			 * Just open the sink path directly. If we ever need different behaviour
			 * then we patch this bit when we know what we need.
			 */
			if ((sinkfd = open(argv[optind], O_WRONLY)) == -1)
				err(EXIT_FAILURE, "Failed to open %s", argv[optind]);

			optind++;
		}

		basic_source.source = sourcefd;
		source.ops = &basic_source_ops;
		source.ctx = &basic_source;

		sysrq_sink.sink = sinkfd;
		sink.ops = &sysrq_sink_ops;
		sink.ctx = &sysrq_sink;
	}

	/* Set up the dbus sink actions if requested via --sink-actions=dbus */
	if (sink_actions && !strcmp("dbus", sink_actions)) {
		sd_bus *bus;
		int rc;

		rc = sd_bus_open_system(&bus);
		if (rc < 0) {
			errx(EXIT_FAILURE, "Failed to connect to the system bus: %s",
			       strerror(-rc));
		}

		dbus_source.bus = bus;
		dbus_source.pfds[DBUS_SOURCE_PFD_SOURCE].fd = sourcefd;
		dbus_source.pfds[DBUS_SOURCE_PFD_SOURCE].events = POLLIN;
		source.ops = &dbus_source_ops;
		source.ctx = &dbus_source;

		dbus_sink.bus = bus;
		sink.ops = &dbus_sink_ops;
		sink.ctx = &dbus_sink;
	}

	/* Check we're done with the command-line */
	if (optind < argc)
		errx(EXIT_FAILURE, "Found %d unexpected arguments", argc - optind);

	if (!(source.ops && source.ctx))
		errx(EXIT_FAILURE, "Invalid source configuration");

	if (!(sink.ops && sink.ctx))
		errx(EXIT_FAILURE, "Unrecognised sink: %s", sink_actions);

	/* Trigger the actions on the sink when we receive an event from the source */
	if (process(&source, &sink) < 0)
		errx(EXIT_FAILURE, "Failure while processing command stream");

	return 0;
}
