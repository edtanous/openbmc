liboemhandlerdir = ${libdir}/ipmid-providers
liboemhandler_LTLIBRARIES = liboemhandler.la

liboemhandler_la_SOURCES = oemhandler.cpp \
                           host-interface.cpp

liboemhandler_la_LDFLAGS = $(SYSTEMD_LIBS)\
                           $(SDBUSPLUS_LIBS) \
                           $(OPENPOWER_DBUS_INTERFACES_LIBS) \
                           $(PHOSPHOR_LOGGING_LIBS) \
                           -version-info 0:0:0 -shared

liboemhandler_la_CXXFLAGS = $(SYSTEMD_CFLAGS) \
                            $(SDBUSPLUS_CFLAGS) \
                            $(OPENPOWER_DBUS_INTERFACES_CFLAGS) \
                            $(PHOSPHOR_LOGGING_CFLAGS) \
                            $(AM_CXXFLAGS)
