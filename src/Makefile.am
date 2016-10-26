bin_PROGRAMS = calcelestial

calcelestial_SOURCES = calcelestial.c objects.c helpers.c formatter.c
calcelestial_LDADD = -lm

OBJS = sun moon mars neptune jupiter mercury uranus saturn venus pluto

if GEONAMES_SUPPORT
  noinst_PROGRAMS = geonames

  geonames_SOURCES = geonames_main.c geonames.c
  geonames_LDADD = $(DEPS_GEONAMES_LIBS)

  calcelestial_SOURCES += geonames.c
  calcelestial_LDADD += $(DEPS_GEONAMES_LIBS)

  AM_CFLAGS = $(DEPS_GEONAMES_CFLAGS)
endif

links:
	for OBJ in $(OBJS); do \
		ln -s calcelestial $$OBJ; \
	done
