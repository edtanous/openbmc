AM_CPPFLAGS = -I$(top_srcdir)/ \
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
	handler_unittest
TESTS = $(check_PROGRAMS)

handler_unittest_SOURCES = handler_unittest.cpp
handler_unittest_LDADD = $(PHOSPHOR_LOGGING_LIBS) \
	$(top_builddir)/handler.o
handler_unittest_CXXFLAGS = $(PHOSPHOR_LOGGING_CFLAGS)
