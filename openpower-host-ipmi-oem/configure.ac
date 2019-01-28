# Initialization
AC_PREREQ([2.69])
AC_INIT([openpower-host-ipmi-oem], [1.0], [https://github.com/openbmc/openpower-host-ipmi-oem/issues])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign dist-xz])
AM_SILENT_RULES([yes])
AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES([SYSTEMD], [libsystemd >= 221])

# Checks for header files.
AC_CHECK_HEADER(systemd/sd-bus.h, ,[AC_MSG_ERROR([Could not find systemd/sd-bus.h...systemd development package required])])
AC_CHECK_HEADER(host-ipmid/ipmid-api.h, ,[AC_MSG_ERROR([Could not find host-ipmid/ipmid-api.h...host-ipmid package required])])
PKG_CHECK_MODULES([OPENPOWER_DBUS_INTERFACES], [openpower-dbus-interfaces],,
                  [AC_MSG_ERROR([Could not find openpower-dbus-interfaces...openbmc/openpower-dbus-interfaces package required])])
PKG_CHECK_MODULES([SDBUSPLUS], [sdbusplus],,\
    AC_MSG_ERROR(["Requires sdbusplus package."]))
PKG_CHECK_MODULES([PHOSPHOR_LOGGING], [phosphor-logging],,\
    AC_MSG_ERROR(["Requires phosphor-logging package."]))

# Check for sdbus++ tool
AC_PATH_PROG([SDBUSPLUSPLUS], [sdbus++])
AS_IF([test "x$SDBUSPLUSPLUS" == "x"], AC_MSG_ERROR([Cannot find sdbus++]))

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX_17([noext])
AX_APPEND_COMPILE_FLAGS([-Wall -Werror], [CXXFLAGS])

# Checks for library functions.
LT_INIT([disable-static shared])

# Check/set gtest specific functions.
AX_PTHREAD([GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=1"],[GTEST_CPPFLAGS="-GTEST_HAS_PTHREAD=0"])
AC_SUBST(GTEST_CPPFLAGS)

AC_ARG_ENABLE([oe-sdk],
    AS_HELP_STRING([--enable-oe-sdk], [Link testcases absolutely against OE SDK so they can be ran within it.])
)
AC_ARG_VAR(OECORE_TARGET_SYSROOT,
    [Path to the OE SDK SYSROOT])
AS_IF([test "x$enable_oe_sdk" == "xyes"],
    AS_IF([test "x$OECORE_TARGET_SYSROOT" == "x"],
          AC_MSG_ERROR([OECORE_TARGET_SYSROOT must be set with --enable-oe-sdk])
    )
    AC_MSG_NOTICE([Enabling OE-SDK at $OECORE_TARGET_SYSROOT])
    [
        testcase_flags="-Wl,-rpath,\${OECORE_TARGET_SYSROOT}/lib"
        testcase_flags="${testcase_flags} -Wl,-rpath,\${OECORE_TARGET_SYSROOT}/usr/lib"
        testcase_flags="${testcase_flags} -Wl,-dynamic-linker,`find \${OECORE_TARGET_SYSROOT}/lib/ld-*.so | sort -r -n | head -n1`"
    ]
    AC_SUBST([OESDK_TESTCASE_FLAGS], [$testcase_flags])
)

# Host object name in the D-Bus
AC_ARG_VAR(HOST_NAME, [The Host name in the object path])
AS_IF([test "x$HOST_NAME" == "x"],
      [HOST_NAME="host"])
AC_DEFINE_UNQUOTED([HOST_NAME], ["$HOST_NAME"], [The Host name in the object path])

# Service dbus object manager
AC_ARG_VAR(CONTROL_HOST_OBJ_MGR, [The Control Host D-Bus Object Manager])
AS_IF([test "x$CONTROL_HOST_OBJ_MGR" == "x"],
      [CONTROL_HOST_OBJ_MGR="/org/open_power/control"])
AC_DEFINE_UNQUOTED([CONTROL_HOST_OBJ_MGR], ["$CONTROL_HOST_OBJ_MGR"], [The Control Host D-Bus Object Manager])

AS_IF([test "x$INVSENSOR_YAML_GEN" == "x"], [INVSENSOR_YAML_GEN="inventory-sensor-example.yaml"])
INVSENSORGEN="$PYTHON ${srcdir}/scripts/inventory-sensor.py -i $INVSENSOR_YAML_GEN"
AC_SUBST(INVSENSORGEN)

AC_DEFINE(MAPPER_BUS_NAME, "xyz.openbmc_project.ObjectMapper",
          [The object mapper bus name])

AC_DEFINE(MAPPER_OBJ, "/xyz/openbmc_project/object_mapper",
          [The object mapper object path])

AC_DEFINE(MAPPER_IFACE, "xyz.openbmc_project.ObjectMapper",
          [The object mapper interface])

# Create configured output.
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
