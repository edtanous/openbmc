# Copyright © 2016 IBM Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
	 socket-handler.c \
	 console-socket.c

obmc_console_client_SOURCES = \
	 console-client.c \
	 console-server.h \
	 console-socket.c \
	 util.c

config_test_CPPFLAGS = -DCONFIG_TEST -DSYSCONFDIR=\"\"
config_test_SOURCES = \
	config.c
