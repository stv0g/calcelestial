	/**
 * Main routine
 *
 * Does parsing of command line options and start calculation
 *
 * @copyright	2012 Steffen Vogel
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <post@steffenvogel.de>
 * @link	http://www.steffenvogel.de/2012/03/14/cron-jobs-fur-sonnenauf-untergang/
 */
/*
 * This file is part of sun
 *
 * sun is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sun is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sun. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <float.h>
#include <math.h>
#include <sys/time.h>

#include "../config.h"
#include "sun.h"

static struct option long_options[] = {
	{"twilight",	required_argument, 0, 't'},
	{"date",	required_argument, 0, 'd'},
	{"format",	required_argument, 0, 'f'},
	{"lat",		required_argument, 0, 'a'},
	{"lon",		required_argument, 0, 'o'},
#ifdef GEONAMES_SUPPORT
	{"query",	required_argument, 0, 'q'},
#endif
	{"zone",	required_argument, 0, 'z'},
	{"help",	no_argument,	   0, 'h'},
	{"version",	no_argument,	   0, 'v'},
	{0}
};

static char *long_options_descs[] = {
	"use special twilight (nautic|civil|astro)",
	"calculcate for specified date (eg. 2011-12-25)",
	"output format (eg. %H:%M:%S)",
	"geographical latitude (-90° to 90°)",
	"geographical longitude (-180° to 180°)",
#ifdef GEONAMES_SUPPORT
	"query geonames.org for geographical position",
#endif
	"use timezone for output",
	"show this help",
	"show version"
};

void version () {
	printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
}

void usage() {
	printf("usage: sun mode [options]\n\n");
	printf("  mode is one of: rise, set, noon, daytime, nighttime\n\n");
	printf("  following OPTIONS are available\n");

	struct option *op = long_options;
	char **desc = long_options_descs;
	while (op->name && desc) {
		printf("\t-%c, --%s\t%s\n", op->val, op->name, *desc);
		op++;
		desc++;
	}

	printf("\nA combination of --lat, --lon or --query is required!\n");
	printf("Please report bugs to: %s\n", PACKAGE_BUGREPORT);
}

char * strreplace(char *subject, char *search, char *replace) {
	int new_len = strlen(subject);
	int search_len = strlen(search);
	int replace_len = strlen(replace);
	char *tmp;

	for (tmp = strstr(subject, search); tmp != NULL; tmp = strstr(tmp + search_len, search)) {
		new_len += replace_len - search_len;
	}

	char *old = subject;
	char *new = malloc(new_len);

	new[0] = '\0'; /* empty string */
	for (tmp = strstr(subject, search); tmp != NULL; tmp = strstr(tmp + search_len, search)) {
		new_len = strlen(new);

		strncpy(new + new_len, old, tmp - old);
		strcpy(new + new_len + (tmp - old), replace);
		old = tmp + search_len;
	}

	strcpy(new + strlen(new), old);

	return new;
}

int main(int argc, char *argv[]) {
	/* default options */
	double twilight = -50.0 / 60; /* 50 Bogenminuten; no twilight, normal sunset/rise */
	char *format = "%H:%M";
	char *query = NULL;
	bool error = false;
	int timezone = 0;

	enum mode mode;
	struct tm date;
	struct coords pos = { INFINITY, INFINITY };

	/* default date: now */
	time_t t;
	time(&t);
	localtime_r(&t, &date);

	/* default timezone: system */
	struct timezone tz;
	if (gettimeofday(NULL, &tz) == 0) {
		timezone = -tz.tz_minuteswest / 60.0;
	}

	/* parse command line arguments */
	while (1) {
		int optidx;
		int c = getopt_long(argc-1, argv+1, "hvt:d:f:a:o:q:z:", long_options, &optidx);

		/* detect the end of the options. */
		if (c == -1) break;

		switch (c) {
			case 't':
				if (strcmp(optarg, "civil") == 0) {
					twilight = -6.0;
				}
				else if (strcmp(optarg, "nautic") == 0) {
					twilight = -12.0;
				}
				else if (strcmp(optarg, "astro") == 0) {
					twilight = -18.0;
				}
				else {
					fprintf(stderr, "invalid twilight: %s\n", optarg);
					error = true;
				}
				break;

			case 'd':
				if (strptime(optarg, "%Y-%m-%d", &date) == 0) {
					fprintf(stderr, "invalid date: %s\n", optarg);
					error = true;
				}
				break;

			case 'f':
				format = strdup(optarg);
				break;

			case 'a':
				pos.lat = strtod(optarg, NULL);
				break;

			case 'o':
				pos.lon = strtod(optarg, NULL);
				break;
#ifdef GEONAMES_SUPPORT
			case 'q':
				query = strdup(optarg);
				break;
#endif

			case 'z':
				timezone = atoi(optarg);
				break;

			case 'v':
				version();
				return EXIT_SUCCESS;

			case 'h':
				usage();
				return EXIT_SUCCESS;

			case '?':
			default:
				error = true;
		}
	}

	/* parse command */
	if (argc < 2) {
		fprintf(stderr, "mode is missing\n");
		error = true;
	}
	else if (strcmp(argv[1], "rise") == 0) mode = RISE;
	else if (strcmp(argv[1], "set") == 0) mode = SET;
	else if (strcmp(argv[1], "noon") == 0) mode = NOON;
	else if (strcmp(argv[1], "daytime") == 0) mode = DAYTIME;
	else if (strcmp(argv[1], "nighttime") == 0) mode = NIGHTTIME;
	else {
		fprintf(stderr, "invalid mode: %s\n", argv[1]);
		error = true;
	}

#ifdef GEONAMES_SUPPORT
	/* lookup place at http://geonames.org */
	if (query && geonames_lookup(query, &pos, NULL, 0) != 0) {
		fprintf(stderr, "failed to lookup location: %s\n", query);
		error = true;
	}
#endif

	/* validate coordinates */
	if (pos.lat == INFINITY) {
		fprintf(stderr, "latitude is missing\n");
		error = true;
	}
	else {
		if (fabs(pos.lat) > 90) {
			fprintf(stderr, "invalid latitude: %.4f\n", pos.lat);
			error = true;
		}
		else if (fabs(pos.lat) > 65) {
			fprintf(stderr, "attention: results may be inaccurate! (delta_T > 5min)\n");
		}
	}

	if (pos.lon == INFINITY) {
		fprintf(stderr, "longitude is missing\n");
		error = true;
	}
	else if (fabs(pos.lon) > 180) {
		fprintf(stderr, "invalid longitude: %.4f\n", pos.lon);
		error = true;
	}

	/* abort on errors */
	if (error) {
		printf("\n");
		usage();
		return EXIT_FAILURE;
	}

#ifdef DEBUG
	char date_str[64];
	strftime(date_str, 64, "%Y-%m-%d", &date);
	printf("calculate for: %s\n", date_str);
	printf("for position: %f, %f\n", pos.lat, pos.lon);
	printf("with twilight: %f\n", twilight);
#endif

	/* start the calculation */
	struct tm result = sun(pos, date, mode, twilight, timezone);

	char result_str[64];

	/* workaround for unix timestamps and 'struct tm's limitation on years > 1990 */
	if (mode == DAYTIME || mode == NIGHTTIME) {
		if (strstr(format, "%s") != NULL) {
			result.tm_year = 70;
			result.tm_mon = 0;
			result.tm_mday = 1;
			result.tm_hour++;

			char buffer[16];
			sprintf(buffer, "%lu", mktime(&result));

#ifdef DEBUG
			printf("buffer: %s\n", buffer);
#endif

			format = strreplace(format, "%s", buffer);

			result.tm_hour--;

#ifdef DEBUG
			printf("format: %s\n", format);
#endif
		}

		result.tm_year = -1900;
		result.tm_mon = -1;
		result.tm_mday = 0;
	}

	strftime(result_str, 64, format, &result);
	printf("%s\n", result_str);

	return EXIT_SUCCESS;
}
