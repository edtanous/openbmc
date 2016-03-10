
#include <endian.h>
#include <err.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#include <linux/types.h>

#include "console-server.h"

struct log_handler {
	struct handler	handler;
	struct console	*console;
	int		fd;
	size_t		size;
	size_t		maxsize;
	int		pagesize;
};


static const char *filename = "uart.log";
static const size_t logsize = 16 * 1024;

static struct log_handler *to_log_handler(struct handler *handler)
{
	return container_of(handler, struct log_handler, handler);
}

static int log_init(struct handler *handler, struct console *console)
{
	struct log_handler *lh = to_log_handler(handler);
	int rc;

	lh->console = console;
	lh->maxsize = logsize;
	lh->pagesize = 4096;
	lh->size = 0;

	lh->fd = open(filename, O_RDWR | O_CREAT, 0644);
	if (lh->fd < 0) {
		warn("Can't open log buffer file %s", filename);
		return -1;
	}
	rc = ftruncate(lh->fd, 0);
	if (rc) {
		warn("Can't truncate file %s", filename);
		close(lh->fd);
		return -1;
	}

	return 0;
}

static int log_trim(struct log_handler *lh, size_t space)
{
	int rc, n_shift_pages, shift_len, shift_start;
	off_t pos;
	void *buf;

	pos = lseek(lh->fd, 0, SEEK_CUR);

	n_shift_pages = (space + lh->pagesize - 1) / lh->pagesize;
	shift_start = n_shift_pages * lh->pagesize;
	shift_len = pos - (n_shift_pages * lh->pagesize);

	buf = mmap(NULL, pos, PROT_READ | PROT_WRITE, MAP_SHARED, lh->fd, 0);
	if (buf == MAP_FAILED)
		return -1;

	memmove(buf, buf + shift_start, shift_len);

	munmap(buf, pos);

	lh->size = shift_len;
	rc = ftruncate(lh->fd, lh->size);
	if (rc)
		warn("failed to truncate file");
	lseek(lh->fd, 0, SEEK_END);

	return 0;

}

static int log_data(struct handler *handler, uint8_t *buf, size_t len)
{
	struct log_handler *lh = to_log_handler(handler);
	int rc;

	if (len > lh->maxsize) {
		buf += len - lh->maxsize;
		len = lh->maxsize;
	}

	if (lh->size + len > lh->maxsize) {
		rc = log_trim(lh, len);
		if (rc)
			return rc;
	}

	rc = write_buf_to_fd(lh->fd, buf, len);
	if (rc)
		return rc;

	lh->size += len;

	return 0;
}

static void log_fini(struct handler *handler)
{
	struct log_handler *lh = to_log_handler(handler);
	close(lh->fd);
}

static struct log_handler log_handler = {
	.handler = {
		.name		= "log",
		.init		= log_init,
		.data_in	= log_data,
		.fini		= log_fini,
	},
};

console_register_handler(&log_handler.handler);

