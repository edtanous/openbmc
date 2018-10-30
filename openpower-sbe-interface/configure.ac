# Initialization
AC_PREREQ([2.69])
AC_INIT([openpower-sbe-interface],[1.0],[https://github.com/openbmc/openpower-sbe-interface/issues])
AC_CONFIG_SRCDIR([sbe_interfaces.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign dist-xz])
AM_SILENT_RULES([yes])
AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AM_PROG_AR

LT_INIT

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX_14([noext])
AX_APPEND_COMPILE_FLAGS([-Wall -Werror], [CFLAGS])
AX_APPEND_COMPILE_FLAGS([-Wall -Werror], [CXXFLAGS])

# Create configured output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
