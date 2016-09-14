liboemhandlerdir = ${libdir}/ipmid-providers
liboemhandler_LTLIBRARIES = liboemhandler.la
liboemhandler_la_SOURCES = oemhandler.cpp
liboemhandler_la_LDFLAGS = $(SYSTEMD_LIBS) -version-info 0:0:0 -shared
liboemhandler_la_CXXFLAGS = $(SYSTEMD_CFLAGS) $(AM_CXXFLAGS)
