#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(calcelestial, 0.1, post@steffenvogel.de)
AC_CONFIG_SRCDIR([src/calcelestial.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Automake
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])

# geonames.org support
AC_ARG_ENABLE(
    [geonames], 
    [AS_HELP_STRING([--enable-geonames], [enable support geonames.org geocoding (def=yes)])],
    [enable_geonames=$enableval],
    [enable_geonames=yes]
)

AM_CONDITIONAL([GEONAMES_SUPPORT], [test x"$enable_geonames" = x"yes"])
if test x"$enable_geonames" = x"yes"; then
    AC_DEFINE([GEONAMES_SUPPORT], [], [Geonames geocoding])
    PKG_CHECK_MODULES([DEPS_GEONAMES], [libcurl >= 7.21, json >= 0.9])
fi


AC_DEFINE([GEONAMES_SUPPORT], [1], [compile with geonames.org lookup capabilities])


# debug compilation support
AC_ARG_ENABLE(
    [debug],
    [AS_HELP_STRING([--enable-debug], [enable debug data generation (def=no)])],
    [debug=$enableval],
    [debug=no]
)

if test x"$debug" = x"yes"; then
    AC_DEFINE([DEBUG], [], [enable debugging])
    AM_CXXFLAGS="$AM_CXXFLAGS -g -Wall -Werror -Wno-uninitialized -O0 -fno-omit-frame-pointer -Woverloaded-virtual -Wno-system-headers"
else
    AM_CXXFLAGS="$AM_CXXFLAGS -O3"
fi

AC_OUTPUT
