AUTOMAKE_OPTIONS = foreign

sbin_PROGRAMS = console-server
bin_PROGRAMS = console-client

AM_CFLAGS = -Wall -Wextra -Werror

console_server_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

console_server_SOURCES = \
	 console-server.c \
	 console-server.h \
	 util.c \
	 log-handler.c \
	 socket-handler.c

console_client_SOURCES = \
	 console-client.c \
	 console-server.h \
	 util.c
