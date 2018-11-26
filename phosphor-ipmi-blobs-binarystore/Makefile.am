AM_DEFAULT_SOURCE_EXT = .cpp

libbinarystoredir = ${libdir}/ipmid-providers
libbinarystore_LTLIBRARIES = libbinarystore.la
libbinarystore_la_SOURCES = main.cpp \
			    handler.cpp
libbinarystore_la_LDFLAGS = $(PHOSPHOR_LOGGING_LIBS) \
			    -version-info 0:0:0 -shared
libbinarystore_la_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS) \
			     -flto

SUBDIRS = . test
