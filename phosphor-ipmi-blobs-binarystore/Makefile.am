PROTOC = protoc

PROTOS_PATH = proto
vpath %.proto $(PROTOS_PATH)

AM_DEFAULT_SOURCE_EXT = .cpp
SUFFIXES = .proto .pb.cc

libbinarystoredir = ${libdir}/ipmid-providers
libbinarystore_LTLIBRARIES = libbinarystore.la
libbinarystore_la_SOURCES = main.cpp \
			    binaryblob.pb.cc \
			    binarystore.cpp \
			    sys.cpp \
			    sys_file.cpp \
			    handler.cpp

libbinarystore_la_LDFLAGS = $(PHOSPHOR_LOGGING_LIBS) \
			    -version-info 0:0:0 -shared \
			    -lprotobuf

libbinarystore_la_CXXFLAGS = -I$(srcdir) $(PHOSPHOR_LOGGING_CFLAGS) \
			     -flto

binaryblob.pb.h binaryblob.pb.cc: $(srcdir)/$(PROTOS_PATH)/binaryblob.proto
	$(PROTOC) -I$(srcdir)/$(PROTOS_PATH) --cpp_out=. $<

BUILT_SOURCES = binaryblob.pb.h binaryblob.pb.cc

SUBDIRS = . test

ACLOCAL_AMFLAGS = -I m4
