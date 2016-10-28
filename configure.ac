#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(calcelestial, 0.4, post@steffenvogel.de)
AC_CONFIG_SRCDIR([src/calcelestial.c])
AC_CONFIG_HEADERS([config.h])

# Automake
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O

# Custom switches
# geonames.org support
AC_ARG_ENABLE(
    [geonames], 
    [AS_HELP_STRING([--enable-geonames], [enable support geonames.org geocoding (def=yes)])],
    [enable_geonames=$enableval],
    [enable_geonames=yes]
)

# debug compilation support
AC_ARG_ENABLE(
    [debug],
    [AS_HELP_STRING([--enable-debug], [enable debug data generation (def=no)])],
    [enable_debug=$enableval],
    [enable_debug=no]
)

# Defines
if test x"$enable_geonames" = x"yes"; then
    AC_DEFINE([WITH_GEONAMES], [1], [compile with geonames.org lookup capabilities])
fi

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
AC_CHECK_LIB([nova],[ln_get_version],[],[AC_MSG_ERROR([Couldn't find libnova])])

if test x"$enable_geonames" = x"yes"; then
    AC_CHECK_LIB([curl],[curl_version],[],[AC_MSG_ERROR([Couldn't find libcurl])])
    AC_CHECK_LIB([json-c],[json_c_version],[has_lib_jsonc=yes])
    AC_CHECK_LIB([json],[json_object_get],[has_lib_json=yes])
    AC_CHECK_LIB([db],[db_create],[],[AC_MSG_ERROR([Couldn't find libdb])])
    
    if test x"$has_lib_json" != x"yes" && test x"$has_lib_jsonc" != x"yes"; then
       AC_MSG_ERROR([Couldn't find libjson0 or libjson-c library])
    fi
fi

# Checks for header files.
AC_CHECK_HEADERS([db.h],[],[AC_MSG_ERROR([Couldn't find or include Berkeley DB header])])
AC_CHECK_HEADERS([libnova/libnova.h],[],[AC_MSG_ERROR([Couldn't find or include libnova headers])])

if test x"$enable_geonames" = x"yes"; then
    AC_CHECK_HEADERS([curl/curl.h],[],[AC_MSG_ERROR([Couldn't find or include libcurl headers])])

    AC_CHECK_HEADERS([json/json.h],[has_include_json=yes])
    AC_CHECK_HEADERS([json-c/json.h],[has_include_jsonc=yes])

    if test x"$has_include_json" != x"yes" && test x"$has_include_jsonc" != x"yes"; then
       AC_MSG_ERROR([Couldn't find libjson0 or libjson-c headers])
    fi
fi

# Conditionals
AM_CONDITIONAL([WITH_GEONAMES], [test x"$enable_geonames" = x"yes"])
AM_CONDITIONAL([DEBUG], [test x"$enable_debug" = x"yes"])
AM_CONDITIONAL([HAS_JSONC], [test x"$has_lib_jsonc" = x"yes"])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])

AC_OUTPUT
