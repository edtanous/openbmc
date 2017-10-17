liboemhandlerdir = ${libdir}/ipmid-providers
liboemhandler_LTLIBRARIES = liboemhandler.la

liboemhandler_la_SOURCES = oemhandler.cpp \
                           host-interface.cpp \
                           org/open_power/OCC/Metrics/error.cpp

BUILT_SOURCES = org/open_power/OCC/Metrics/error.hpp \
                org/open_power/OCC/Metrics/error.cpp

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

org/open_power/OCC/Metrics/error.hpp: ${top_srcdir}/org/open_power/OCC/Metrics.errors.yaml
	@mkdir -p `dirname $@`
	$(SDBUSPLUSPLUS) -r $(top_srcdir) error exception-header org.open_power.OCC.Metrics > $@

org/open_power/OCC/Metrics/error.cpp: ${top_srcdir}/org/open_power/OCC/Metrics.errors.yaml
	@mkdir -p `dirname $@`
	$(SDBUSPLUSPLUS) -r $(top_srcdir) error exception-cpp org.open_power.OCC.Metrics > $@
