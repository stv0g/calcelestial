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

const char* username = "libastro";
const char* request_url_tpl = "http://api.geonames.org/search?name=%s&maxRows=1&username=%s&type=json&orderby=relevance";

static size_t json_parse_callback(void *contents, size_t size, size_t nmemb, void *userp) {
	static struct json_tokener *jtok;
	static struct json_object *jobj;
	size_t realsize = size * nmemb;

	/* initialize tokener */
	if (jtok == NULL) {
		jtok = json_tokener_new();
		jtok->err = json_tokener_continue;
	}

	if (jtok->err == json_tokener_continue) {
#ifdef DEBUG
		printf("debug: received chunk: %zu * %zu = %zu bytes\r\n", size, nmemb, realsize);
		printf("   %.*s\r\n", (int) realsize, (char *) contents);
#endif

		jobj = json_tokener_parse_ex(jtok, (char *) contents, realsize);

		if (jtok->err == json_tokener_success) {
			*(struct json_object **) userp = jobj;
			json_tokener_free(jtok);
		}
		else if (jtok->err != json_tokener_continue) {
			const char *err = json_tokener_error_desc(json_tokener_get_error(jtok));
			fprintf(stderr, "json parse error: %s\r\n", err);
			*(void **) userp = NULL;
			json_tokener_free(jtok);
		}
	}

	return realsize;
}

int geonames_lookup(const char *place, struct ln_lnlat_posn *result, char *name, int n) {
#ifdef GEONAMES_CACHE_SUPPORT
	int ret;
	
	ret = geonames_cache_db(1, place, result);
	if (ret == 0) {
		strncpy(name, place, n);
#ifdef DEBUG
		printf("debug: using cached geonames entry\n");
#endif
		return 0;
	}
#endif

	CURL *ch;
	CURLcode res;

	struct json_object *jobj;

	/* setup curl */
	ch = curl_easy_init();
	if (!ch)
		return -1;

	/* prepare url */
	int len = strlen(place) + strlen(request_url_tpl) + 1;
	char *request_url = malloc(len);
	if (!request_url)
		return -2;

	snprintf(request_url, len, request_url_tpl, place, username);

#ifdef DEBUG
	printf("debug: request url: %s\r\n", request_url);
#endif

	curl_easy_setopt(ch, CURLOPT_URL, request_url);
	curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, json_parse_callback);
	curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) &jobj);
	curl_easy_setopt(ch, CURLOPT_USERAGENT, "libastro/1.0");

	/* perform request */
	res = curl_easy_perform(ch);

	/* always cleanup */ 
	curl_easy_cleanup(ch);

	if (res != CURLE_OK) {
		fprintf(stderr, "request failed: %s\n", curl_easy_strerror(res));
		return -1;
	}

	if (jobj) {
		int ret = geonames_parse(jobj, result, name, n);
		if (!ret) {
#ifdef GEONAMES_CACHE_SUPPORT
			geonames_cache_db(0, place, result);
#ifdef DEBUG
			printf("debug: storing cache entry\n");
#endif
#endif
		}

		return ret;
	}
	else
		return -1;
}

int geonames_parse(struct json_object *jobj, struct ln_lnlat_posn *result, char *name, int n) {
	int results = json_object_get_int(json_object_object_get(jobj, "totalResultsCount"));
	if (results == 0)
		return -1;

	struct json_object *jobj_place = json_object_array_get_idx(json_object_object_get(jobj, "geonames"), 0);
	result->lat = json_object_get_double(json_object_object_get(jobj_place, "lat"));
	result->lng = json_object_get_double(json_object_object_get(jobj_place, "lng"));

	if (name && n > 0)
		strncpy(name, json_object_get_string(json_object_object_get(jobj_place, "name")), n);

	return 0;
}

int geonames_cache_db(int lookup, const char *place, struct ln_lnlat_posn *coords) {
	int ret;
	DB *dbp;
	DBT key, data;

	char filename[256];
	char *place_lower;
	
	place_lower = strdup(place);
	if (!place_lower)
		return -1;
	
	for (char *p = place_lower; *p; ++p)
		*p = tolower(*p);	

	snprintf(filename, sizeof(filename), "%s/%s", getenv("HOME"), GEONAMES_CACHE_FILE);

	dbp = dbopen(filename, O_RDWR | O_CREAT, 0664, DB_BTREE, NULL);
	if (!dbp) {
		fprintf(stderr, "dbopen: %s\n", strerror(errno));
		exit (1);
	}
	
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	key.data = place_lower;
	key.size = strlen(place_lower) + 1;
	
	if (lookup) {
		ret = dbp->get(dbp, &key, &data, 0);
		if (ret)
			goto err;

#ifdef DEBUG
		printf("debug: cache key retrieved: %s => %f %f.\n", (char *) key.data,
			((struct ln_lnlat_posn *) data.data)->lat,
			((struct ln_lnlat_posn *) data.data)->lng);
#endif
		if (data.size != sizeof(struct ln_lnlat_posn))
			goto err;
			
		memcpy(coords, data.data, sizeof(struct ln_lnlat_posn));
	}
	else {
		data.data = coords;
		data.size = sizeof(struct ln_lnlat_posn);
		
		ret = dbp->put(dbp, &key, &data, 0);
		if (ret)
			goto err;

#ifdef DEBUG
		printf("debug: cache key stored: %s => %f %f.\n", (char *) key.data,
			((struct ln_lnlat_posn *) data.data)->lat,
			((struct ln_lnlat_posn *) data.data)->lng);
#endif
	}
	
err:	
	dbp->close(dbp);

	return ret;
}
