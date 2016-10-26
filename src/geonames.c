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
		printf("got chunk: %d * %d = %d bytes\r\n", size, nmemb, realsize);
#endif

		jobj = json_tokener_parse_ex(jtok, (char *) contents, realsize);

		if (jtok->err == json_tokener_success) {
			*(struct json_object **) userp = jobj;
			json_tokener_free(jtok);
		}
		else if (jtok->err != json_tokener_continue) {
			fprintf(stderr, "parse error: %s\r\n", json_tokener_get_error(jtok));
			*(void **) userp = NULL;
			json_tokener_free(jtok);
		}
	}

	return realsize;
}

int geonames_lookup(const char *place, struct ln_lnlat_posn *result, char *name, int n) {

#ifdef GEONAMES_CACHE_SUPPORT
	if (geonames_cache_lookup(place, result, name, n) == EXIT_SUCCESS) {
#ifdef DEBUG
		printf("using cached geonames entry\n");
#endif
		return EXIT_SUCCESS;
	}
#endif

	CURL *ch;
	CURLcode res;

	struct json_object *jobj;

	/* setup curl */
	ch = curl_easy_init();
	if (!ch) return -1;

	/* prepare url */
	int len = strlen(place) + strlen(request_url_tpl) + 1;
	char *request_url = malloc(len);
	if (!request_url) {
		return -2;
	}

	snprintf(request_url, len, request_url_tpl, place, username);

#ifdef DEBUG
	printf("request url: %s\r\n", request_url);
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
		return EXIT_FAILURE;
	}

	if (jobj) {
		int ret = geonames_parse(jobj, result, name, n);
		if (ret == EXIT_SUCCESS) {
#ifdef GEONAMES_CACHE_SUPPORT
			geonames_cache_store(place, result, name, n);
#ifdef DEBUG
			printf("storing cache entry\n");
#endif
#endif
		}

		return ret;
	}
	else {
		return EXIT_FAILURE;
	}
}

int geonames_parse(struct json_object *jobj, struct ln_lnlat_posn *result, char *name, int n) {
	int results = json_object_get_int(json_object_object_get(jobj, "totalResultsCount"));
	if (results == 0) {
		return EXIT_FAILURE;
	}

	struct json_object *jobj_place = json_object_array_get_idx(json_object_object_get(jobj, "geonames"), 0);
	result->lat = json_object_get_double(json_object_object_get(jobj_place, "lat"));
	result->lng = json_object_get_double(json_object_object_get(jobj_place, "lng"));

	if (name && n > 0) {
		strncpy(name, json_object_get_string(json_object_object_get(jobj_place, "name")), n);
	}

	return EXIT_SUCCESS;
}

int geonames_cache_lookup(const char *place, struct ln_lnlat_posn *result, char *name, int n) {
	/* create filename */
	char filename[256];
	snprintf(filename, sizeof(filename), "%s/%s", getenv("HOME"), GEONAMES_CACHE_FILE);

	FILE *file = fopen(filename, "r"); /* should check the result */
	if (file == NULL) {
		return EXIT_FAILURE;
	}

	char line[256];
	while (fgets(line, sizeof(line), file)) {
		/* replace newline at the end */
		char *end = strchr(line, '\n');
		if (end == NULL) {
			return EXIT_FAILURE;
		}
		else {
			*end = '\0';
		}

		char *tok;
		int col;
		for (col = 0, tok = strtok(line, "\t"); tok != NULL; tok = strtok(NULL, "\t")) {
			switch (col) {
				case 0:
					if (strcasecmp(tok, place) != 0) {
						continue; /* skip row */
					}
					break;

				case 1:
					result->lat = strtod(tok, NULL);
					break;

				case 2:
					result->lng = strtod(tok, NULL);
					break;

				case 3:
					strncpy(name, tok, n);
					fclose(file);
					return EXIT_SUCCESS; /* found! */
			}
			col++;
		}
	}

	fclose(file);
	return 1; /* not found */
}

int geonames_cache_store(const char *place, struct ln_lnlat_posn *result, char *name, int n) {
	/* create filename */
	char filename[256];
	snprintf(filename, sizeof(filename), "%s/%s", getenv("HOME"), GEONAMES_CACHE_FILE);

	FILE* file = fopen(filename, "a+"); /* should check the result */
	if (file == NULL) {
		return EXIT_FAILURE;
	}

	/* build cache entry */
	char line[256];
	snprintf(line, sizeof(line), "%s\t%.5f\t%.5f\t%s\n", place, result->lat, result->lng, name);

	if (fputs(line, file) == EOF) {
		fclose(file);
		return EXIT_FAILURE;
	}

	fclose(file);
	return EXIT_SUCCESS;
}
