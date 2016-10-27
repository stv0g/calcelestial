bin_PROGRAMS = calcelestial

calcelestial_SOURCES = calcelestial.c objects.c formatter.c
calcelestial_LDADD = -lm

OBJS = sun moon mars neptune jupiter mercury uranus saturn venus pluto

if WITH_GEONAMES
  noinst_PROGRAMS = geonames

  geonames_SOURCES = geonames_main.c geonames.c formatter.c

  calcelestial_SOURCES += geonames.c
endif

if HAS_JSONC
  calcelestial_LDADD += -ljson-c
  geonames_LDADD = -ljson-c
else
  calcelestial_LDADD += -ljson
  geonames_LDADD = -ljson
endif


links:
	for OBJ in $(OBJS); do \
		ln -s calcelestial $$OBJ; \
	done
