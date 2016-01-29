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
#include <termios.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/poll.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static const char esc_str[] = { '\r', '~', '.' };

struct console_ctx {
	const char	*tty_kname;
	char		*tty_sysfs_devnode;
	char		*tty_dev;
	int		tty_fd;
	int		console_fd_in;
	int		console_fd_out;
	bool		console_is_tty;
	struct termios	orig_termios;
	int		esc_str_pos;
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
static int tty_find_device(struct console_ctx *ctx)
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
			"/sys/class/tty/%s", ctx->tty_kname);
	if (rc < 0)
		return -1;

	tty_device_tty_dir = realpath(tty_class_device_link, NULL);
	if (rc < 0) {
		warn("Can't query sysfs for device %s", ctx->tty_kname);
		goto out_free;
	}

	rc = asprintf(&tty_device_reldir, "%s/../../", tty_device_tty_dir);
	if (rc < 0)
		goto out_free;

	ctx->tty_sysfs_devnode = realpath(tty_device_reldir, NULL);
	if (!ctx->tty_sysfs_devnode)
		warn("Can't find parent device for %s", ctx->tty_kname);


	/* todo: lookup from major/minor info in sysfs, in case udev has
	 * renamed us */
	rc = asprintf(&ctx->tty_dev, "/dev/%s", ctx->tty_kname);
	if (rc < 0)
		goto out_free;

	rc = 0;

out_free:
	free(tty_class_device_link);
	free(tty_device_tty_dir);
	free(tty_device_reldir);
	return rc;
}

/**
 * Open and initialise the serial device
 */
static int tty_init_io(struct console_ctx *ctx)
{

	ctx->tty_fd = open(ctx->tty_dev, O_RDWR);
	if (ctx->tty_fd <= 0) {
		warn("Can't open tty %s", ctx->tty_dev);
		return -1;
	}

	/* Disable character delay. We may want to later enable this when
	 * we detect larger amounts of data
	 */
	fcntl(ctx->tty_fd, F_SETFL, FNDELAY);

	return 0;
}

/*
 * Setup our console channel for IO: use stdin/stdout, and if we're on a TTY,
 * put it in canonical mode
 */
static int console_init_io(struct console_ctx *ctx)
{
	struct termios termios;
	int rc;

	ctx->console_fd_in = STDIN_FILENO;
	ctx->console_fd_out = STDOUT_FILENO;
	ctx->console_is_tty = isatty(ctx->console_fd_in);

	if (!ctx->console_is_tty)
		return 0;

	rc = tcgetattr(ctx->console_fd_in, &termios);
	if (rc) {
		warn("Can't get terminal attributes for console");
		return -1;
	}
	memcpy(&ctx->orig_termios, &termios, sizeof(ctx->orig_termios));
	cfmakeraw(&termios);

	rc = tcsetattr(ctx->console_fd_in, TCSANOW, &termios);
	if (rc) {
		warn("Can't set terminal attributes for console");
		return -1;
	}

	return 0;
}

static int console_process_input(struct console_ctx *ctx,
		uint8_t *buf, size_t len)
{
	unsigned long i;
	uint8_t e;

	e = esc_str[ctx->esc_str_pos];

	for (i = 0; i < len; i++) {
		if (buf[i] == e) {
			ctx->esc_str_pos++;
			if (ctx->esc_str_pos == ARRAY_SIZE(esc_str))
				return 1;
			e = esc_str[ctx->esc_str_pos];
		} else {

			ctx->esc_str_pos = 0;
		}
	}
	return 0;
}

static void console_restore_termios(struct console_ctx *ctx)
{
	if (ctx->console_is_tty)
		tcsetattr(ctx->console_fd_in, TCSANOW, &ctx->orig_termios);
}

static int write_buf_to_fd(int fd, uint8_t *buf, size_t len)
{
	size_t pos;
	ssize_t rc;

	for (pos = 0; pos < len; pos += rc) {
		rc = write(fd, buf + pos, len - pos);
		if (rc <= 0) {
			warn("Write error");
			return -1;
		}
	}

	return 0;
}

int run_console(struct console_ctx *ctx)
{
	struct pollfd pollfds[2];
	int rc, len;

	pollfds[0].fd = ctx->tty_fd;
	pollfds[0].events = POLLIN;
	pollfds[1].fd = ctx->console_fd_in;
	pollfds[1].events = POLLIN;

	for (;;) {
		uint8_t buf[4096];

		rc = poll(pollfds, 2, -1);
		if (rc < 0) {
			warn("poll error");
			return -1;
		}

		if (pollfds[0].revents) {
			rc = read(ctx->tty_fd, buf, sizeof(buf));
			if (rc <= 0) {
				warn("Error reading from tty device");
				return -1;
			}
			rc = write_buf_to_fd(ctx->console_fd_out, buf, rc);
			if (rc < 0)
				return -1;
		}
		if (pollfds[1].revents) {
			rc = read(ctx->console_fd_in, buf, sizeof(buf));
			if (rc == 0)
				return 0;

			if (rc <= 0) {
				warn("Error reading from console");
				return -1;
			}
			len = rc;
			rc = console_process_input(ctx, buf, len);
			if (rc) {
				rc = 0;
				return 0;
			}
			rc = write_buf_to_fd(ctx->tty_fd, buf, len);
			if (rc < 0)
				return -1;
		}
	}
}

static const struct option options[] = {
	{ "device",	required_argument,	0, 'd'},
	{ },
};

int main(int argc, char **argv)
{
	struct console_ctx *ctx;
	int rc;

	ctx = malloc(sizeof(struct console_ctx));
	memset(ctx, 0, sizeof(*ctx));

	for (;;) {
		int c, idx;

		c = getopt_long(argc, argv, "d", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			ctx->tty_kname = optarg;
			break;

		case 'h':
		case '?':
			usage(argv[0]);
			break;
		}
	}

	if (!ctx->tty_kname) {
		fprintf(stderr,
			"Error: No TTY device specified (use --device)\n");
		return EXIT_FAILURE;
	}

	rc = tty_find_device(ctx);
	if (rc)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;

	rc = tty_init_io(ctx);
	if (rc)
		return EXIT_FAILURE;

	rc = console_init_io(ctx);
	if (rc)
		return EXIT_FAILURE;

	rc = run_console(ctx);

	console_restore_termios(ctx);

	free(ctx->tty_sysfs_devnode);
	free(ctx->tty_dev);
	free(ctx);

	return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
