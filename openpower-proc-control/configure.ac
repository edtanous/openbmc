AC_PREREQ([2.69])
AC_INIT([openpower-proc-control], [1.0], [https://github.com/openbmc/openpower-proc-control/issues])
AC_LANG([C++])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign dist-xz])
AM_SILENT_RULES([yes])

# Checks for programs
AC_PROG_CXX
AC_PROG_INSTALL #Checks/sets the install variable to be used
AC_PROG_MAKE_SET

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX_17([noext])
AX_APPEND_COMPILE_FLAGS([-Wall -Werror], [CXXFLAGS])

# Checks for library functions
LT_INIT

# Check/set gtest specific functions.
AX_PTHREAD([GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=1"],[GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=0"])
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

#CHIPS can be passed in a from a recipe, or it will default to P9 and openfsi
AC_ARG_VAR(CHIPS, [The list of chips to build the procedures for])

AC_ARG_ENABLE([p9], AS_HELP_STRING([--enable-p9], [Use internal host initialisation procedures for POWER9 processors]))
AS_IF([test "$enable_p9" == "yes"], [CHIPS+=" p9 common"])

AC_ARG_ENABLE([openfsi], AS_HELP_STRING([--enable-openfsi], [Support Openfsi]))
AS_IF([test "$enable_openfsi" == "yes"], [CHIPS+=" openfsi"])

AC_ARG_ENABLE([phal], AS_HELP_STRING([--enable-phal], [Use host processor initialisation procedures from PHAL])])

AS_IF([ test "$enable_phal" == "yes" && test "$enable_p9" == "yes"],
      [ AC_MSG_ERROR([Internal POWER9 and PHAL-provided procedures conflict, please configure only one])]
     )

AS_IF([test "$enable_phal" == "yes"],
      [
       AX_ABSOLUTE_HEADER([libipl.h])
       if test x"$gl_cv_absolute_libipl_h" == "x" ; then
               AC_MSG_ERROR([Cannot find libipl.h path])
       fi

       AC_CHECK_LIB([ipl], [ipl_init])
       if test x"$ac_cv_lib_ipl_ipl_init" != "xyes" ; then
               AC_MSG_ERROR([IPL library not found])
       fi

       CHIPS+=" phal common"
       AC_CONFIG_FILES([set-spi-mux.service])
      ]
     )
AM_CONDITIONAL([ENABLE_PHAL], [test "$enable_phal" == "yes"])

AS_IF([test "x$CHIPS" == "x"], [CHIPS="p9 openfsi common"])

AC_CONFIG_FILES([Makefile.generated],
    [${srcdir}/gen_makefile.sh ${srcdir} "$myChips" > Makefile.generated],
    [myChips="$CHIPS"])

AC_CHECK_LIB([pdbg], [thread_sreset_all])

PKG_CHECK_MODULES([SYSTEMD], [libsystemd >= 221])
PKG_CHECK_MODULES([PHOSPHOR_LOGGING], [phosphor-logging])
PKG_CHECK_MODULES([PHOSPHOR_DBUS_INTERFACES], [phosphor-dbus-interfaces])

PKG_PROG_PKG_CONFIG
AC_ARG_WITH([systemdsystemunitdir],
     [AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files])],,
     [with_systemdsystemunitdir=auto])
AS_IF([test "x$with_systemdsystemunitdir" = "xyes" -o "x$with_systemdsystemunitdir" = "xauto"], [
     def_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)

     AS_IF([test "x$def_systemdsystemunitdir" = "x"],
   [AS_IF([test "x$with_systemdsystemunitdir" = "xyes"],
    [AC_MSG_ERROR([systemd support requested but pkg-config unable to query systemd package])])
    with_systemdsystemunitdir=no],
   [with_systemdsystemunitdir="$def_systemdsystemunitdir"])])
AS_IF([test "x$with_systemdsystemunitdir" != "xno"],
      [AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])])

AC_CONFIG_FILES([Makefile test/Makefile])
AC_CONFIG_FILES([pcie-poweroff@.service])
AC_CONFIG_FILES([xyz.openbmc_project.Control.Host.NMI.service])
AC_CONFIG_FILES([op-stop-instructions@.service])
AC_OUTPUT
