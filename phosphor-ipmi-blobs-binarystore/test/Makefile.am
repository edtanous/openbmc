AM_CPPFLAGS = -I$(top_srcdir)/ \
	-I$(builddir)
	$(GTEST_CFLAGS) \
	$(GMOCK_CFLAGS)
AM_CXXFLAGS = \
	$(GTEST_MAIN_CFLAGS)
AM_LDFLAGS = \
	$(GMOCK_LIBS) \
	$(GTEST_MAIN_LIBS) \
	$(OESDK_TESTCASE_FLAGS)

# Run all 'check' test programs
check_PROGRAMS = \
	handler_open_unittest \
	handler_readwrite_unittest \
	handler_unittest
TESTS = $(check_PROGRAMS)

handler_unittest_SOURCES = handler_unittest.cpp
handler_unittest_LDADD = $(PHOSPHOR_LOGGING_LIBS) \
	$(top_builddir)/handler.o \
	$(top_builddir)/binarystore.o \
	$(top_builddir)/libbinarystore_la-binaryblob.pb.o \
	-lprotobuf
handler_unittest_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS)

handler_open_unittest_SOURCES = handler_open_unittest.cpp
handler_open_unittest_LDADD = $(PHOSPHOR_LOGGING_LIBS) \
	$(top_builddir)/handler.o \
	$(top_builddir)/binarystore.o \
	$(top_builddir)/libbinarystore_la-binaryblob.pb.o \
	-lprotobuf
handler_open_unittest_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS)

handler_readwrite_unittest_SOURCES = handler_readwrite_unittest.cpp
handler_readwrite_unittest_LDADD = $(PHOSPHOR_LOGGING_LIBS) \
	$(top_builddir)/handler.o \
	$(top_builddir)/binarystore.o \
	$(top_builddir)/libbinarystore_la-binaryblob.pb.o \
	-lprotobuf
handler_readwrite_unittest_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS)
