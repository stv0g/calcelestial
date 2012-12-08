#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json/json.h>

#include "geonames.h"

const char* username = "libastro";
const char* request_url_tpl = "http://api.geonames.org/search?name=%s&maxRows=1&username=%s&type=json&orderby=relevance";

struct memory_block {
	char *address;
	size_t size;
};

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
			fprintf(stderr, "parse error: %s\r\n", json_tokener_errors[jtok->err]);
			*(void **) userp = NULL;
			json_tokener_free(jtok);
		}
	}

	return realsize;
}

int geonames_lookup(const char *place, struct coords *result) {
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
		return geonames_parse(jobj, result);;
	}
	else {
		return EXIT_FAILURE;
	}
}

int geonames_parse(struct json_object *jobj, struct coords *result) {
	struct json_object *jobj_place = json_object_array_get_idx(json_object_object_get(jobj, "geonames"), 0);

	result->lat = json_object_get_double(json_object_object_get(jobj_place, "lat"));
	result->lon = json_object_get_double(json_object_object_get(jobj_place, "lng"));

	/* cleanup */
	json_object_put(jobj);

	return EXIT_SUCCESS;
}
