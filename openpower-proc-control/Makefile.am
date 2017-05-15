AM_DEFAULT_SOURCE_EXT = .cpp

sbin_PROGRAMS = \
	openpower-proc-control

openpower_proc_control_SOURCES = \
	proc_control.cpp \
	cfam_access.cpp \
	filedescriptor.cpp \
	registration.cpp \
	targeting.cpp \
	openpower_procedures.cpp

CLEANFILES = openpower_procedures.cpp

openpower_proc_control_LDFLAGS = $(PHOSPHOR_LOGGING_LIBS) -lstdc++fs \
                                 $(PHOSPHOR_DBUS_INTERFACES_LIBS)
openpower_proc_control_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS) \
                                  $(PHOSPHOR_DBUS_INTERFACES_CFLAGS)

SUBDIRS = test

-include Makefile.generated
