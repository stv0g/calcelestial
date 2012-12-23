#ifndef _GEONAMES_H_
#define _GEONAMES_H_

#include <json/json.h>

#define GEONAMES_CACHE_SUPPORT 1
#define GEONAMES_CACHE_FILE ".geonames.cache" /* in users home dir */

struct coords {
	double lat;
	double lon;
};

int geonames_lookup(const char *place, struct coords *coords, char *name, int n);
int geonames_cache_lookup(const char *place, struct coords *result, char *name, int n);
int geonames_cache_store(const char *place, struct coords *result, char *name, int n);
int geonames_parse(struct json_object *jobj, struct coords *result, char *name, int n);

#endif /* _GEONAMES_H_ */
