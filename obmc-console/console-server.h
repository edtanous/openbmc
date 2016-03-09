
struct console;

#include <poll.h>
#include <stdint.h>

enum {
	HANDLER_OK = 0,
	HANDLER_EXIT,
};

struct handler {
	const char	*name;
	int		(*init)(struct handler *handler,
				struct console *console);
	int		(*init_poll)(struct handler *hander,
				struct pollfd *pollfd);
	int		(*poll_event)(struct handler *handler,
				int events);
	int		(*data_in)(struct handler *handler,
				uint8_t *buf, size_t len);
	void		(*fini)(struct handler *handler);
};

#define __handler_name(n) __handler_  ## n
#define  _handler_name(n) __handler_name(n)

#define console_register_handler(h) \
	static const \
		__attribute__((section("handlers"))) \
		__attribute__((used)) \
		struct handler * _handler_name(__COUNTER__) = h;

int console_data_out(struct console *console, const uint8_t *data, size_t len);

/* utils */
int write_buf_to_fd(int fd, const uint8_t *buf, size_t len);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define offsetof(type, member) \
	((unsigned long)&((type *)NULL)->member)

#define container_of(ptr, type, member) \
	((type *)((void *)((ptr) - offsetof(type, member))))

#define BUILD_ASSERT(c) \
	do { \
		char __c[(c)?1:-1] __attribute__((unused)); \
	} while (0)
