AM_CPPFLAGS = -I$(top_srcdir)

# Run all 'check' test programs
TESTS = $(check_PROGRAMS)

# Basic test suite for hostcontrol interfaces
check_PROGRAMS = utest
utest_CPPFLAGS = -Igtest $(GTEST_CPPFLAGS) $(AM_CPPFLAGS)

utest_CXXFLAGS = $(PTHREAD_CFLAGS) \
				 $(PHOSPHOR_LOGGING_CFLAGS) \
				 $(PHOSPHOR_DBUS_INTERFACES_CFLAGS)

utest_LDFLAGS = -lgtest_main -lgtest \
				$(PTHREAD_LIBS) \
				$(OESDK_TESTCASE_FLAGS) \
				$(PHOSPHOR_LOGGING_LIBS) \
				$(PHOSPHOR_DBUS_INTERFACES_LIBS)

utest_SOURCES = utest.cpp
utest_LDADD = $(top_srcdir)/targeting.cpp $(top_srcdir)/filedescriptor.cpp
