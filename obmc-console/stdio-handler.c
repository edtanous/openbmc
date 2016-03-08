
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "console-server.h"

struct stdio_handler {
	struct handler	handler;
	struct console	*console;
	int		fd_in;
	int		fd_out;
	bool		is_tty;
	struct termios	orig_termios;
	int		esc_str_pos;
};

static struct stdio_handler *to_stdio_handler(struct handler *handler)
{
	return container_of(handler, struct stdio_handler, handler);
}


static const uint8_t esc_str[] = { '\r', '~', '.' };




/*
 * Setup our console channel for IO: use stdin/stdout, and if we're on a TTY,
 * put it in canonical mode
 */
static int stdio_init(struct handler *handler, struct console *console)
{
	struct stdio_handler *sh = to_stdio_handler(handler);
	struct termios termios;
	int rc;

	sh->console = console;
	sh->fd_in = STDIN_FILENO;
	sh->fd_out = STDOUT_FILENO;
	sh->is_tty = isatty(sh->fd_in);

	if (!sh->is_tty)
		return 0;

	rc = tcgetattr(sh->fd_in, &termios);
	if (rc) {
		warn("Can't get terminal attributes for console");
		return -1;
	}
	memcpy(&sh->orig_termios, &termios, sizeof(sh->orig_termios));
	cfmakeraw(&termios);

	rc = tcsetattr(sh->fd_in, TCSANOW, &termios);
	if (rc) {
		warn("Can't set terminal attributes for console");
		return -1;
	}

	return 0;
}

static int stdio_init_poll(struct handler *handler, struct pollfd *pollfd)
{
	struct stdio_handler *sh = to_stdio_handler(handler);
	pollfd->fd = sh->fd_in;
	pollfd->events = POLLIN;
	return 0;
}

static int stdio_data(struct handler *handler, uint8_t *buf, size_t len)
{
	struct stdio_handler *sh = to_stdio_handler(handler);
	return write_buf_to_fd(sh->fd_out, buf, len);
}

static int process_input(struct stdio_handler *sh,
		uint8_t *buf, size_t len)
{
	unsigned long i;
	uint8_t e;

	e = esc_str[sh->esc_str_pos];

	for (i = 0; i < len; i++) {
		if (buf[i] == e) {
			sh->esc_str_pos++;
			if (sh->esc_str_pos == ARRAY_SIZE(esc_str))
				return 1;
			e = esc_str[sh->esc_str_pos];
		} else {
			console_data_out(sh->console,
					esc_str, sh->esc_str_pos);
			sh->esc_str_pos = 0;
		}
	}
	return 0;
}

static int stdio_poll_event(struct handler *handler, int events)
{
	struct stdio_handler *sh = to_stdio_handler(handler);
	uint8_t buf[4096];
	ssize_t len;
	int rc;

	if (!(events & POLLIN))
		return 0;

	len = read(sh->fd_in, buf, sizeof(buf));
	if (len <= 0)
		return -1;

	rc = process_input(sh, buf, len);
	if (rc)
		return HANDLER_EXIT;

	rc = console_data_out(sh->console, buf, len);
	if (rc < 0)
		return -1;

	return 0;
}

static void stdio_fini(struct handler *handler)
{
	struct stdio_handler *sh = to_stdio_handler(handler);
	if (sh->is_tty)
		tcsetattr(sh->fd_in, TCSANOW, &sh->orig_termios);
}

static struct stdio_handler stdio_handler = {
	.handler = {
		.name		= "stdio",
		.init		= stdio_init,
		.init_poll	= stdio_init_poll,
		.poll_event	= stdio_poll_event,
		.data_in	= stdio_data,
		.fini		= stdio_fini,
	},
};

console_register_handler(&stdio_handler.handler);

