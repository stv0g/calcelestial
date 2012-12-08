#ifndef _GEONAMES_H_
#define _GEONAMES_H_

//#define DEBUG 1

#include <json/json.h>

struct coords {
	double lat;
	double lon;
};

int geonames_lookup(const char *place, struct coords *coords);
int geonames_parse(struct json_object *jobj, struct coords *result);

#endif /* _GEONAMES_H_ */
