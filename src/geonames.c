/**
 * Geonames.org lookup routines
 *
 * @copyright	2012 Steffen Vogel
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <post@steffenvogel.de>
 * @link	https://www.noteblok.net/2012/03/14/cron-jobs-fur-sonnenauf-untergang/
 */
/*
 * This file is part of calcelestial
 *
 * calcelestial is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * calcelestial is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with calcelestial. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <db.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

#include <curl/curl.h>
#include <json-c/json.h>
#include <libnova/libnova.h>

#include "../config.h"
#include "geonames.h"
#include "formatter.h"

static const char* url_tpl = "http://api.geonames.org/search?q=%s&maxRows=1&username=libastro&type=json&orderby=relevance";
static const char* url_tz_tpl = "http://api.geonames.org/timezoneJSON?lat=%.6f&lng=%.6f&username=libastro";

struct string {
	char *ptr;
	size_t len;
};

struct ctx_latlng {
	struct ln_lnlat_posn *coords;
	char *name;
	size_t namelen;
};

struct ctx_tz {
	int *gmt_offset;
	char *tzid;
	size_t tzidlen;
};

#ifdef GEONAMES_CACHE_SUPPORT
enum cache_op { STORE, LOOKUP, DELETE };

static int cache(enum cache_op op, const char *url, struct string *s)
{
	int ret;
	DB *dbp;
	DBT key, data;

	char filename[256];
	snprintf(filename, sizeof(filename), "%s/%s", getenv("HOME"), GEONAMES_CACHE_FILE);
	
	ret = db_create(&dbp, NULL, 0);
	if (ret) {
		fprintf(stderr, "Error: db: %s\n", db_strerror(ret));
		return ret;
	}

	ret = dbp->open(dbp, NULL, filename, NULL, DB_BTREE, DB_CREATE, 0);
	if (ret) {
		fprintf(stderr, "Error: db: %s\n", db_strerror(ret));
		return ret;
	}
	
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	key.data = (void *) url;
	key.size = strlen(url) + 1;
	
	switch (op) {
		case LOOKUP:
			ret = dbp->get(dbp, NULL, &key, &data, 0);
			if (ret)
				goto err;

#ifdef DEBUG
			printf("Debug: cache key retrieved: %s => %s.\n", (char *) key.data, (char *) data.data);
#endif /* DEBUG */
			s->ptr = malloc(data.size);
			s->len = data.size;
			if (!s->ptr) {
				fprintf(stderr, "malloc() failed: %s\n", strerror(errno));
				goto err;
			}
			
			memcpy(s->ptr, data.data, data.size);
			break;
		
		case STORE:
			data.data = s->ptr;
			data.size = s->len;
		
			ret = dbp->put(dbp, NULL, &key, &data, 0);
			if (ret) {
				fprintf(stderr, "Error: db: %s\n", db_strerror(ret));
				goto err;
			}
#ifdef DEBUG
			printf("Debug: cache key stored: %s => %s\n", (char *) key.data, (char *) data.data);
#endif /* DEBUG */
			break;

		case DELETE: {
			// TODO
		}
	}
	
err:	
	dbp->close(dbp, 0);

	return ret;
}
#endif /* GEONAMES_CACHE_SUPPORT */

static size_t writefunction(void *contents, size_t size, size_t nmemb, void *userp)
{
	struct string *s = userp;
	
	size_t new_len = s->len + size*nmemb;

	s->ptr = realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	
	memcpy(s->ptr + s->len, contents, size*nmemb);
	
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

static int request_json(const char *url, int (*parser)(struct json_object *jobj, void *ctx), void *ctx)
{
	int ret, cached;

	CURL *ch;
	CURLcode res;

	struct json_object *jobj;
	enum json_tokener_error error;
	
	struct string s = { 0 };
	
#ifdef GEONAMES_CACHE_SUPPORT
	cached = cache(LOOKUP, url, &s) == 0;
	if (cached)
		goto cached;
#endif /* GEONAMES_CACHE_SUPPORT */

	/* Setup curl */
	ch = curl_easy_init();
	if (!ch)
		return -1;

#ifdef DEBUG
	printf("Debug: request url: %s\r\n", url);
#endif /* DEBUG */

	curl_easy_setopt(ch, CURLOPT_URL, url);
	curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, writefunction);
	curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) &s);
	curl_easy_setopt(ch, CURLOPT_USERAGENT, "libastro/1.0");

	/* perform request */
	res = curl_easy_perform(ch);

	/* always cleanup */ 
	curl_easy_cleanup(ch);

	if (res != CURLE_OK) {
		fprintf(stderr, "Error: request failed: %s\n", curl_easy_strerror(res));
		return -1;
	}
	
#ifdef DEBUG
	printf("Debug: request completed: %s\r\n", s.ptr);
#endif /* DEBUG */

cached:	
	jobj = json_tokener_parse_verbose(s.ptr, &error);
	if (!jobj) {
#ifdef DEBUG
		printf("Debug: failed to parse json: %s\r\n", json_tokener_error_desc(error));
#endif /* DEBUG */
	}
		
		
	ret = parser(jobj, ctx);
	if (!ret && !cached) {
#ifdef GEONAMES_CACHE_SUPPORT
		cache(STORE, url, &s);
#endif /* GEONAMES_CACHE_SUPPORT */
	}
#ifdef DEBUG
	else
		printf("Debug: failed to parse: %d\n", ret);
#endif /* DEBUG */

	json_object_put(jobj);

	return ret;
}

static int parser_tz(struct json_object *jobj, void *userp)
{
	struct ctx_tz *ctx = userp;
	struct json_object *jobj_offset, *jobj_tzid;
	
	json_bool exists;
	
	exists = json_object_object_get_ex(jobj, "gmtOffset", &jobj_offset);
	if (!exists)
		return -1;
	
	exists = json_object_object_get_ex(jobj, "timezoneId", &jobj_tzid);
	if (!exists)
		return -1;
	
	*ctx->gmt_offset = json_object_get_int(jobj_offset);
	
	if (ctx->tzid)
		strncpy(ctx->tzid, json_object_get_string(jobj_tzid), ctx->tzidlen);
	
	return 0;
}

static int parser_latlng(struct json_object *jobj, void *userp)
{
	struct ctx_latlng *ctx = userp;
	struct json_object *jobj_count, *jobj_geonames, *jobj_place, *jobj_lat, *jobj_lng, *jobj_name;
	int results;
	
	json_bool exists;
	
	exists = json_object_object_get_ex(jobj, "totalResultsCount", &jobj_count);
	if (!exists)
		return -1;

	exists = json_object_object_get_ex(jobj, "geonames", &jobj_geonames);
	if (!exists)
		return -2;
	
	results = json_object_get_int(jobj_count);
	if (results == 0)
		return -3;

	jobj_place = json_object_array_get_idx(jobj_geonames, 0);
	if (!jobj_place)
		return -4;
	
	exists = json_object_object_get_ex(jobj_place, "lat", &jobj_lat);
	if (!exists)
		return -5;

	exists = json_object_object_get_ex(jobj_place, "lng", &jobj_lng);
	if (!exists)
		return -6;
	
	exists = json_object_object_get_ex(jobj_place, "name", &jobj_name);
	if (!exists)
		return -7;

	ctx->coords->lat = json_object_get_double(jobj_lat);
	ctx->coords->lng = json_object_get_double(jobj_lng);

	if (ctx->name)
		strncpy(ctx->name, json_object_get_string(jobj_name), ctx->namelen);

	return 0;
}

int geonames_lookup_tz(struct ln_lnlat_posn coords, int *gmt_offset, char *tzid, size_t tzidlen)
{
	char url[256];
	struct ctx_tz ctx = {
		.gmt_offset = gmt_offset,
		.tzid = tzid,
		.tzidlen = tzidlen
	};
	
	snprintf(url, sizeof(url), url_tz_tpl, coords.lat, coords.lng);

	return request_json(url, parser_tz, &ctx);
}

int geonames_lookup_latlng(const char *place, struct ln_lnlat_posn *coords, char *name, size_t namelen)
{
	int ret;
	char url[256];
	struct ctx_latlng ctx = {
		.coords = coords,
		.name = name,
		.namelen = namelen
	};
	
	//char *escaped_place = curl_escape(place, 0);
	char *escaped_place = strrepl(place, " ", "+");
	
	snprintf(url, sizeof(url), url_tpl, escaped_place);

	ret = request_json(url, parser_latlng, &ctx);
	
	//curl_free(escaped_place);
	free(escaped_place);
	
	return ret;
}