#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(calcelestial, 0.4, post@steffenvogel.de)
AC_CONFIG_SRCDIR([src/calcelestial.c])
AC_CONFIG_HEADERS([config.h])

# Extra args
AC_ARG_ENABLE(
    [geonames], 
    [AS_HELP_STRING([--enable-geonames], [enable support geonames.org geocoding (def=yes)])],
    [enable_geonames=$enableval],
    [enable_geonames=yes]
)

AC_ARG_ENABLE(
    [debug],
    [AS_HELP_STRING([--enable-debug], [enable debug data generation (def=no)])],
    [debug=$enableval],
    [debug=no]
)

AM_CONDITIONAL([GEONAMES_SUPPORT], [test x"$enable_geonames" = x"yes"])

if test x"$enable_geonames" = x"yes"; then
    AC_DEFINE([GEONAMES_SUPPORT], [1], [compile with geonames.org lookup capabilities])
    PKG_CHECK_MODULES([DEPS_GEONAMES], [libcurl >= 7.21, json-c >= 0.11])
fi

if test x"$debug" = x"yes"; then
    AC_DEFINE([DEBUG], [], [enable debugging])
    AM_CFLAGS="$AM_CFLAGS -g -Wall -Werror -Wno-uninitialized -O0 -fno-omit-frame-pointer"
else
    AM_CFLAGS="$AM_CFLAGS -O3"
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
AC_CHECK_LIB([nova],[ln_get_version],[],[AC_MSG_ERROR([Couldn't find libnova])])

if test x"$enable_geonames" = x"yes"; then
    AC_CHECK_LIB([curl],[curl_version],[],[AC_MSG_ERROR([Couldn't find libcurl])])
    AC_CHECK_LIB([json-c],[json_c_version],[],[AC_MSG_ERROR([Couldn't find libjson-c library])])
    AC_CHECK_LIB([db],[db_create],[],[AC_MSG_ERROR([Couldn't find libdb])])
fi

# Checks for header files.
AC_CHECK_HEADERS([libnova/libnova.h],[],[AC_MSG_ERROR([Couldn't find or include libnova headers])])

if test x"$enable_geonames" = x"yes"; then
    AC_CHECK_HEADERS([curl/curl.h],[],[AC_MSG_ERROR([Couldn't find or include libcurl headers])])
    AC_CHECK_HEADERS([json-c/json.h],[],[AC_MSG_ERROR([Couldn't find libjson-c headers])])
    AC_CHECK_HEADERS([db.h],[],[AC_MSG_ERROR([Couldn't find libdb headers])])
fi

# Automake
AM_INIT_AUTOMAKE([foreign])
AM_PROG_CC_C_O

# Conditionals
AM_CONDITIONAL([WITH_GEONAMES], [test x"$enable_geonames" = x"yes"])
AM_CONDITIONAL([DEBUG], [test x"$enable_debug" = x"yes"])
AM_CONDITIONAL([HAS_JSONC], [test x"$has_lib_jsonc" = x"yes"])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])

AC_OUTPUT
