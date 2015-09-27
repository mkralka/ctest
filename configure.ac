#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

m4_define([base_version], [0.0.1])

AC_PREREQ([2.69])
AC_INIT([C Unit Testing framework], [m4_esyscmd_s([./version.sh rel- ]base_version)], [https://github.com/mkralka/ctest/issues], [ctest], [https://github.com/mkralka/ctest])
AM_INIT_AUTOMAKE([tar-ustar])
LT_INIT

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([.autoconf])

# Libtool/ltdl Support
LT_INIT
LT_CONFIG_LTDL_DIR([libltdl])
LTDL_INIT([convenience])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_DISABLE_FAST_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/cli/Makefile])
AC_CONFIG_FILES([src/exec/Makefile])
AC_CONFIG_FILES([src/tests/Makefile])
AC_CONFIG_FILES([src/tests/fixtures/Makefile])
AC_CONFIG_FILES([src/include/Makefile])
AC_OUTPUT
