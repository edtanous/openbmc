/**
 * Console server process for OpenBMC
 *
 * Copyright © 2016 IBM Corporation <jk@ozlabs.org>
 */

#define _GNU_SOURCE

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/poll.h>

#include "console-server.h"

struct console {
	const char	*tty_kname;
	char		*tty_sysfs_devnode;
	char		*tty_dev;
	int		tty_sirq;
	int		tty_lpc_addr;
	int		tty_fd;
	struct handler	**handlers;
	int		n_handlers;
};

static void usage(const char *progname)
{
	fprintf(stderr,
"usage: %s [options]\n"
"\n"
"Options:\n"
"  --device <TTY>  Use serial device TTY (eg, ttyS0)\n"
"",
		progname);
}

/* populates tty_dev and tty_sysfs_devnode, using the tty kernel name */
static int tty_find_device(struct console *console)
{
	char *tty_class_device_link;
	char *tty_device_tty_dir;
	char *tty_device_reldir;
	int rc;

	rc = -1;
	tty_class_device_link = NULL;
	tty_device_tty_dir = NULL;
	tty_device_reldir = NULL;

	rc = asprintf(&tty_class_device_link,
			"/sys/class/tty/%s", console->tty_kname);
	if (rc < 0)
		return -1;

	tty_device_tty_dir = realpath(tty_class_device_link, NULL);
	if (rc < 0) {
		warn("Can't query sysfs for device %s", console->tty_kname);
		goto out_free;
	}

	rc = asprintf(&tty_device_reldir, "%s/../../", tty_device_tty_dir);
	if (rc < 0)
		goto out_free;

	console->tty_sysfs_devnode = realpath(tty_device_reldir, NULL);
	if (!console->tty_sysfs_devnode)
		warn("Can't find parent device for %s", console->tty_kname);


	/* todo: lookup from major/minor info in sysfs, in case udev has
	 * renamed us */
	rc = asprintf(&console->tty_dev, "/dev/%s", console->tty_kname);
	if (rc < 0)
		goto out_free;

	rc = 0;

out_free:
	free(tty_class_device_link);
	free(tty_device_tty_dir);
	free(tty_device_reldir);
	return rc;
}

static int tty_set_sysfs_attr(struct console *console, const char *name,
		int value)
{
	char *path;
	FILE *fp;
	int rc;

	rc = asprintf(&path, "%s/%s", console->tty_sysfs_devnode, name);
	if (rc < 0)
		return -1;

	fp = fopen(path, "w");
	if (!fp) {
		warn("Can't access attribute %s on device %s",
				name, console->tty_kname);
		rc = -1;
		goto out_free;
	}
	setvbuf(fp, NULL, _IONBF, 0);

	rc = fprintf(fp, "0x%x", value);
	if (rc < 0)
		warn("Error writing to %s attribute of device %s",
				name, console->tty_kname);
	fclose(fp);



out_free:
	free(path);
	return rc;
}

/**
 * Open and initialise the serial device
 */
static int tty_init_io(struct console *console)
{
	if (console->tty_sirq)
		tty_set_sysfs_attr(console, "sirq", console->tty_sirq);
	if (console->tty_lpc_addr)
		tty_set_sysfs_attr(console, "lpc_address",
				console->tty_lpc_addr);
	tty_set_sysfs_attr(console, "enabled", 1);

	console->tty_fd = open(console->tty_dev, O_RDWR);
	if (console->tty_fd <= 0) {
		warn("Can't open tty %s", console->tty_dev);
		return -1;
	}

	/* Disable character delay. We may want to later enable this when
	 * we detect larger amounts of data
	 */
	fcntl(console->tty_fd, F_SETFL, FNDELAY);

	return 0;
}


int console_data_out(struct console *console, const uint8_t *data, size_t len)
{
	return write_buf_to_fd(console->tty_fd, data, len);
}

static void handlers_init(struct console *console)
{
	extern struct handler *__start_handlers, *__stop_handlers;
	struct handler *handler;
	int i;

	console->n_handlers = &__stop_handlers - &__start_handlers;
	console->handlers = &__start_handlers;

	printf("%d handler%s\n", console->n_handlers,
			console->n_handlers == 1 ? "" : "s");

	for (i = 0; i < console->n_handlers; i++) {
		handler = console->handlers[i];

		printf("  %s\n", handler->name);

		if (handler->init)
			handler->init(handler, console);
	}
}

static void handlers_fini(struct console *console)
{
	struct handler *handler;
	int i;

	for (i = 0; i < console->n_handlers; i++) {
		handler = console->handlers[i];
		if (handler->fini)
			handler->fini(handler);
	}
}

static int handlers_data_in(struct console *console, uint8_t *buf, size_t len)
{
	struct handler *handler;
	int i, rc, tmp;

	rc = 0;

	for (i = 0; i < console->n_handlers; i++) {
		handler = console->handlers[i];

		if (!handler->data_in)
			continue;

		tmp = handler->data_in(handler, buf, len);
		if (tmp == HANDLER_EXIT)
			rc = 1;
	}

	return rc;
}

static int handlers_poll_event(struct console *console,
		struct pollfd *pollfds)
{
	struct handler *handler;
	int i, rc, tmp;

	rc = 0;

	for (i = 0; i < console->n_handlers; i++) {
		handler = console->handlers[i];

		if (!handler->poll_event)
			continue;

		tmp = handler->poll_event(handler, pollfds[i].revents);
		if (tmp == HANDLER_EXIT)
			rc = 1;
	}

	return rc;
}

int run_console(struct console *console)
{
	struct handler *handler;
	struct pollfd *pollfds;
	int i, rc;

	pollfds = calloc(console->n_handlers + 1, sizeof(*pollfds));

	pollfds[0].fd = console->tty_fd;
	pollfds[0].events = POLLIN;

	for (;;) {
		uint8_t buf[4096];

		/* init pollers */
		for (i = 0; i < console->n_handlers; i++) {
			handler = console->handlers[i];
			handler->init_poll(handler, &pollfds[i+1]);
		}

		rc = poll(pollfds, console->n_handlers + 1, -1);
		if (rc < 0) {
			warn("poll error");
			return -1;
		}

		if (pollfds[0].revents) {
			rc = read(console->tty_fd, buf, sizeof(buf));
			if (rc <= 0) {
				warn("Error reading from tty device");
				return -1;
			}
			rc = handlers_data_in(console, buf, rc);
			if (rc)
				return 0;
		}

		rc = handlers_poll_event(console, pollfds + 1);
		if (rc)
			return 0;
	}
}
static const struct option options[] = {
	{ "device",	required_argument,	0, 'd'},
	{ "sirq",	required_argument,	0, 's'},
	{ "lpc-addr",	required_argument,	0, 'l'},
	{ },
};

int main(int argc, char **argv)
{
	struct console *console;
	int rc;

	console = malloc(sizeof(struct console));
	memset(console, 0, sizeof(*console));
	rc = -1;

	for (;;) {
		char *endp;
		int c, idx;

		c = getopt_long(argc, argv, "d:s:l:", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			console->tty_kname = optarg;
			break;
		case 'l':
			console->tty_lpc_addr = strtoul(optarg, &endp, 0);
			if (endp == optarg) {
				warnx("Invalid sirq: '%s'", optarg);
				goto out_free;
			}
			break;

		case 's':
			console->tty_sirq = strtoul(optarg, &endp, 0);
			if (endp == optarg) {
				warnx("Invalid sirq: '%s'", optarg);
				goto out_free;
			}
			break;

		case 'h':
		case '?':
			usage(argv[0]);
			rc = 0;
			goto out_free;
		}
	}

	if (!console->tty_kname) {
		fprintf(stderr,
			"Error: No TTY device specified (use --device)\n");
		return EXIT_FAILURE;
	}

	rc = tty_find_device(console);
	if (rc)
		return EXIT_FAILURE;

	rc = tty_init_io(console);
	if (rc)
		return EXIT_FAILURE;

	handlers_init(console);

	rc = run_console(console);

	handlers_fini(console);

out_free:
	free(console->tty_sysfs_devnode);
	free(console->tty_dev);
	free(console);

	return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
