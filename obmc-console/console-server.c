/**
 * Console server process for OpenBMC
 *
 * Copyright © 2016 IBM Corporation <jk@ozlabs.org>
 */

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <signal.h>
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

	struct poller	**pollers;
	int		n_pollers;

	struct pollfd	*pollfds;
};

struct poller {
	struct handler	*handler;
	void		*data;
	poller_fn_t	fn;
	bool		remove;
};

/* we have one extra entry in the pollfds array for the VUART tty */
static const int n_internal_pollfds = 1;

/* state shared with the signal handler */
static bool sigint;

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

	console->pollfds[console->n_pollers].fd = console->tty_fd;
	console->pollfds[console->n_pollers].events = POLLIN;

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

struct poller *console_register_poller(struct console *console,
		struct handler *handler, poller_fn_t poller_fn,
		int fd, int events, void *data)
{
	struct poller *poller;
	int n;

	poller = malloc(sizeof(*poller));
	poller->remove = false;
	poller->handler = handler;
	poller->fn = poller_fn;
	poller->data = data;

	/* add one to our pollers array */
	n = console->n_pollers++;
	console->pollers = realloc(console->pollers,
			sizeof(*console->pollers) * console->n_pollers);

	console->pollers[n] = poller;

	/* increase pollfds array too  */
	console->pollfds = realloc(console->pollfds,
			sizeof(*console->pollfds) *
				(n_internal_pollfds + console->n_pollers));

	/* shift the end pollfds up by one */
	memcpy(&console->pollfds[n+n_internal_pollfds],
			&console->pollfds[n],
			sizeof(*console->pollfds) * n_internal_pollfds);

	console->pollfds[n].fd = fd;
	console->pollfds[n].events = events;

	return poller;
}

void console_unregister_poller(struct console *console,
		struct poller *poller)
{
	int i;

	/* find the entry in our pollers array */
	for (i = 0; i < console->n_pollers; i++)
		if (console->pollers[i] == poller)
			break;

	assert(i < console->n_pollers);

	console->n_pollers--;

	/* remove the item from the pollers array... */
	memmove(&console->pollers[i], &console->pollers[i+1],
			sizeof(*console->pollers)
				* (console->n_pollers - i));

	console->pollers = realloc(console->pollers,
			sizeof(*console->pollers) * console->n_pollers);

	/* ... and the pollfds array */
	memmove(&console->pollfds[i], &console->pollfds[i+1],
			sizeof(*console->pollfds) *
				(n_internal_pollfds + console->n_pollers - i));

	console->pollfds = realloc(console->pollfds,
			sizeof(*console->pollfds) *
				(n_internal_pollfds + console->n_pollers));


	free(poller);
}

static int call_pollers(struct console *console)
{
	struct poller *poller;
	struct pollfd *pollfd;
	enum poller_ret prc;
	int i, rc;

	rc = 0;

	/*
	 * Process poll events by iterating through the pollers and pollfds
	 * in-step, calling any pollers that we've found revents for.
	 */
	for (i = 0; i < console->n_pollers; i++) {
		poller = console->pollers[i];
		pollfd = &console->pollfds[i];

		if (!pollfd->revents)
			continue;

		prc = poller->fn(poller->handler, pollfd->revents,
				poller->data);
		if (prc == POLLER_EXIT)
			rc = -1;
		else if (prc == POLLER_REMOVE)
			poller->remove = true;
	}

	/**
	 * Process deferred removals; restarting each time we unregister, as
	 * the array will have changed
	 */
	for (;;) {
		bool removed = false;

		for (i = 0; i < console->n_pollers; i++) {
			poller = console->pollers[i];
			if (poller->remove) {
				console_unregister_poller(console, poller);
				removed = true;
				break;
			}
		}
		if (!removed)
			break;
	}

	return rc;
}

static void sighandler(int signal)
{
	if (signal == SIGINT)
		sigint = true;
}

int run_console(struct console *console)
{
	sighandler_t sighandler_save;
	int rc;

	sighandler_save = signal(SIGINT, sighandler);

	rc = 0;

	for (;;) {
		uint8_t buf[4096];

		if (sigint) {
			fprintf(stderr, "Received interrupt, exiting\n");
			break;
		}

		rc = poll(console->pollfds,
				console->n_pollers + n_internal_pollfds, -1);
		if (rc < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				warn("poll error");
				break;
			}
		}

		/* process internal fd first */
		BUILD_ASSERT(n_internal_pollfds == 1);

		if (console->pollfds[console->n_pollers].revents) {
			rc = read(console->tty_fd, buf, sizeof(buf));
			if (rc <= 0) {
				warn("Error reading from tty device");
				rc = -1;
				break;
			}
			rc = handlers_data_in(console, buf, rc);
			if (rc)
				break;
		}

		/* ... and then the pollers */
		rc = call_pollers(console);
		if (rc)
			break;
	}

	signal(SIGINT, sighandler_save);

	return rc ? -1 : 0;
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
	int rc, i;

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

	console->pollfds = calloc(n_internal_pollfds,
			sizeof(*console->pollfds));

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
	free(console->pollers);
	free(console->pollfds);
	free(console->tty_sysfs_devnode);
	free(console->tty_dev);
	free(console);

	return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
