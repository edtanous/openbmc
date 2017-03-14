AM_DEFAULT_SOURCE_EXT = .cpp

if WANT_LIBOPENPOWER_DBUS
nobase_include_HEADERS = ## These get filled in by Makefile.interfaces

libopenpower_dbusdir = ${libdir}
libopenpower_dbus_LTLIBRARIES = libopenpower_dbus.la
libopenpower_dbus_la_LDFLAGS = $(SDBUSPLUS_LIBS) -version-info 0:0:0 -shared
libopenpower_dbus_la_CXXFLAGS = $(SDBUSPLUS_CFLAGS)

BUILT_SOURCES = libopenpower_dbus.cpp
CLEANFILES = libopenpower_dbus.cpp

pkgconfiglibdir = ${libdir}/pkgconfig
pkgconfiglib_DATA = openpower-dbus-interfaces.pc

clean-local: clean-dbus

-include Makefile.interfaces
endif

yamldir = ${pkgdatadir}/yaml
nobase_yaml_DATA = ## Filled in by Makefile.yaml

-include Makefile.yaml
