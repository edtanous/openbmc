AM_DEFAULT_SOURCE_EXT = .cpp

sbin_PROGRAMS = \
	openpower-proc-control

openpower_proc_control_SOURCES = \
	proc_control.cpp \
	cfam_access.cpp \
	filedescriptor.cpp \
	targeting.cpp \
	procedures/p9/start_host.cpp \
	procedures/p9/vcs_workaround.cpp

openpower_proc_control_LDFLAGS = $(PHOSPHOR_LOGGING_LIBS) -lstdc++fs
openpower_proc_control_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS)

SUBDIRS = test
