bin_PROGRAMS = sun

sun_SOURCES = sun_main.c sun.c
sun_LDADD = -lm

if GEONAMES_SUPPORT
  TESTS = geonames
  check_PROGRAMS = geonames

  geonames_SOURCES = geonames_main.c geonames.c
  geonames_LDADD = $(DEPS_GEONAMES_LIBS)

  sun_SOURCES += geonames.c
  sun_LDADD += $(DEPS_GEONAMES_LIBS)

  AM_CFLAGS = $(DEPS_GEONAMES_CFLAGS)
endif
