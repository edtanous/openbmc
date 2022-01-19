// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2021 IBM Corp.

/*
 * debug-trigger listens for an external signal that the BMC is in some way unresponsive. When the
 * signal is received it triggers a crash to collect debug data and reboots the system in the hope
 * that it will recover.
 *
 * Usage: debug-trigger [SOURCE] [SINK]
 *
 * Examples:
 *  debug-trigger
 *	Set the source as stdin and the sink as stdout. Useful for testing.
 *
 *  debug-trigger /dev/serio_raw0 /proc/sysrq-trigger
 *	Open /dev/serio_raw0 as the source and /proc/sysrq-trigger as the sink.
 */

#include <err.h>
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

static void process_debug(int sink)
{
	/* https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/admin-guide/sysrq.rst?h=v5.16#n93 */
	static const char action = 'c';
	ssize_t rc;

	sync();

	if ((rc = write(sink, &action, sizeof(action))) == sizeof(action))
		return;

	if (rc == -1) {
		warn("Failed to execute debug command");
	} else {
		warnx("Failed to execute debug command: %zd", rc);
	}
}

static void process_reboot(int sink)
{
	/* https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/admin-guide/sysrq.rst?h=v5.16#n90 */
	static const char action = 'b';
	ssize_t rc;

	sync();

	if ((rc = write(sink, &action, sizeof(action))) == sizeof(action))
		return;

	if (rc == -1) {
		warn("Failed to reboot BMC");
	} else {
		warnx("Failed to reboot BMC: %zd", rc);
	}
}

static int process(int source, int sink)
{
	ssize_t ingress;
	char command;

	while ((ingress = read(source, &command, sizeof(command))) == sizeof(command)) {
		switch (command) {
		case 'D':
			process_debug(sink);
			break;
		case 'R':
			process_reboot(sink);
			break;
		default:
			warnx("Unexpected command: 0x%02x (%c)", command, command);
		}
	}

	if (ingress == -1)
		warn("Failed to read from source");

	return ingress;
}

int main(int argc, char * const argv[])
{
	char devnode[PATH_MAX];
	char *devid;
	int source;
	int sink;

	/* Option processing. Currently nothing implemented, but allows us to use optind */
	while (1) {
		static struct option long_options[] = {
			{0, 0, 0, 0},
		};
		int c;

		c = getopt_long(argc, argv, "", long_options, NULL);
		if (c == -1)
			break;
	}

	/*
	 * The default behaviour sets the source as stdin and the sink as stdout. This allows
	 * trivial testing on the command-line with just a keyboard and without crashing the system.
	 */
	source = 0;
	sink = 1;

	/* Handle the source argument, if any */
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

		if ((source = open(devnode, O_RDONLY)) == -1)
			err(EXIT_FAILURE, "Failed to open %s", devnode);

		optind++;
	}

	/* Handle the sink argument, if any */
	if (optind < argc) {
		/*
		 * Just open the sink path directly. If we ever need different behaviour then we
		 * patch this bit when we know what we need.
		 */
		if ((sink = open(argv[optind], O_WRONLY)) == -1)
			err(EXIT_FAILURE, "Failed to open %s", argv[optind]);

		optind++;
	}

	/* Check we're done with the command-line */
	if (optind < argc)
		err(EXIT_FAILURE, "Found %d unexpected arguments", argc - optind);

	/* Trigger the actions on the sink when we receive an event from the source */
	if (process(source, sink) < 0)
		errx(EXIT_FAILURE, "Failure while processing command stream");

	return 0;
}
