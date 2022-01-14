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
 *	Set the class of sink action(s) to be used. Defaults to 'sysrq'
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
 */

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <linux/reboot.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

const struct debug_sink_ops sysrq_sink_ops = {
	.debug = sysrq_sink_debug,
	.reboot = sysrq_sink_reboot,
};

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

const struct debug_source_ops basic_source_ops = {
	.poll = basic_source_poll,
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
	const char *sink_actions = NULL;
	struct debug_source_basic basic;
	struct debug_sink_sysrq sysrq;
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

		sysrq.sink = sinkfd;
		sink.ops = &sysrq_sink_ops;
		sink.ctx = &sysrq;
	}

	/* Check we're done with the command-line */
	if (optind < argc)
		err(EXIT_FAILURE, "Found %d unexpected arguments", argc - optind);

	basic.source = sourcefd;
	source.ops = &basic_source_ops;
	source.ctx = &basic;

	/* Trigger the actions on the sink when we receive an event from the source */
	if (process(&source, &sink) < 0)
		errx(EXIT_FAILURE, "Failure while processing command stream");

	return 0;
}
