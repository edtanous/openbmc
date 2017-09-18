AM_DEFAULT_SOURCE_EXT = .cpp

sbin_PROGRAMS = \
	openpower-proc-control

openpower_proc_control_SOURCES = \
	proc_control.cpp \
	cfam_access.cpp \
	filedescriptor.cpp \
	registration.cpp \
	targeting.cpp \
	openpower_procedures.cpp \
	ext_interface.cpp

CLEANFILES = openpower_procedures.cpp

openpower_proc_control_LDFLAGS = $(PHOSPHOR_LOGGING_LIBS) \
                                 $(PHOSPHOR_DBUS_INTERFACES_LIBS) \
                                 $(OPENPOWER_DBUS_INTERFACES_LIBS) \
                                 $(SDBUSPLUS_LIBS) \
                                 -lstdc++fs

openpower_proc_control_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS) \
                                  $(PHOSPHOR_DBUS_INTERFACES_CFLAGS) \
                                  $(OPENPOWER_DBUS_INTERFACES_CFLAGS) \
                                  $(SDBUSPLUS_CFLAGS)

SUBDIRS = test

-include Makefile.generated
