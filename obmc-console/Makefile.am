AUTOMAKE_OPTIONS = foreign

sbin_PROGRAMS = obmc-console-server
bin_PROGRAMS = obmc-console-client

check_PROGRAMS = config-test

AM_CFLAGS = -Wall -Wextra -Werror

EXTRA_DIST = obmc-console.conf.sample

obmc_console_server_CPPFLAGS = \
	-DLOCALSTATEDIR=\"$(localstatedir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\"

obmc_console_server_SOURCES = \
	 console-server.c \
	 console-server.h \
	 util.c \
	 config.c \
	 log-handler.c \
	 socket-handler.c

obmc_console_client_SOURCES = \
	 console-client.c \
	 console-server.h \
	 util.c

config_test_CPPFLAGS = -DCONFIG_TEST -DSYSCONFDIR=\"\"
config_test_SOURCES = \
	config.c
