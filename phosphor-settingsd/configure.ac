AC_PREREQ([2.69])
AC_INIT([phosphor-settingsd], [1.0], [https://github.com/openbmc/phosphor-settingsd/issues])
AC_LANG([C++])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign dist-xz])
AM_SILENT_RULES([yes])

AC_PROG_CXX
AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PATH_PYTHON([2.7], [AC_SUBST([PYTHON], [echo "$PYTHON"])], [AC_MSG_ERROR(
    [Could not find python-2.7 installed...python-2.7 is required])])

PKG_CHECK_MODULES([PHOSPHOR_DBUS_INTERFACES], [phosphor-dbus-interfaces],,\
    AC_MSG_ERROR(["Requires phosphor-dbus-interfaces package."]))
PKG_CHECK_MODULES([SDBUSPLUS], [sdbusplus],,
    AC_MSG_ERROR(["Requires sdbusplus package."]))
PKG_CHECK_MODULES([PHOSPHOR_LOGGING], [phosphor-logging],,\
     AC_MSG_ERROR(["Requires phosphor-logging package."]))

LT_INIT

AX_CXX_COMPILE_STDCXX_14([noext])
AX_APPEND_COMPILE_FLAGS([-Wall -Werror], [CXXFLAGS])

AC_ARG_VAR(SETTINGS_BUSNAME, [The Dbus busname to own])
AS_IF([test "x$SETTINGS_BUSNAME" == "x"],\
    [SETTINGS_BUSNAME="xyz.openbmc_project.Settings"])
AC_DEFINE_UNQUOTED([SETTINGS_BUSNAME], ["$SETTINGS_BUSNAME"],\
    [The DBus busname to own])

AS_IF([test "x$SETTINGS_YAML" == "x"], [SETTINGS_YAML="settings_example.yaml"])
SETTINGSGEN="$PYTHON $srcdir/settings.py -i $SETTINGS_YAML"
AC_SUBST(SETTINGSGEN)

AC_ARG_VAR(SETTINGS_PERSIST_PATH, \
    [Path of directory housing persisted settings.])
AS_IF([test "x$SETTINGS_PERSIST_PATH" == "x"], \
    [SETTINGS_PERSIST_PATH="/var/lib/phosphor-settings-manager/settings"])
AC_DEFINE_UNQUOTED([SETTINGS_PERSIST_PATH], ["$SETTINGS_PERSIST_PATH"], \
    [Path of directory housing persisted settings])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
