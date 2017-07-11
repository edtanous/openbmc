include_HEADERS = \
    sbe_interfaces.hpp

noinst_HEADERS = \
    sbe_chipOp_handler.hpp \
    file.hpp

libsbeinterfacedir = ${libdir}/sbe-interface
libsbeinterface_LTLIBRARIES = libsbeinterface.la

libsbeinterface_la_SOURCES = \
    sbe_interfaces.cpp \
    sbe_chipOp_handler.cpp

libsbeinterface_la_LDFLAGS = -version-info 0:0:0 -shared
