libwistronoemdir = ${libdir}/ipmid-providers
libwistronoem_LTLIBRARIES = libwistronoem.la

libwistronoem_la_SOURCES = wistronoem.cpp \
                           smbus.cpp

libwistronoem_la_LDFLAGS = $(SYSTEMD_LIBS)\
                           $(SDBUSPLUS_LIBS) \
                           $(LIBIPMID_LIBS) \
                           $(LIBIPMID_HOST_LIBS) \
                           -version-info 0:0:0 -shared

libwistronoem_la_CXXFLAGS = $(SYSTEMD_CFLAGS) \
                            $(SDBUSPLUS_CFLAGS) \
                            $(LIBIPMID_CFLAGS) \
                            $(LIBIPMID_HOST_CFLAGS) \
                            $(AM_CXXFLAGS)
