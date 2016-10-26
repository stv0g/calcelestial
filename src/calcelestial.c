/**
 * Main routine
 *
 * Does parsing of command line options and start calculation
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

#define _XOPEN_SOURCE 700
#define _BSD_SOURCE 1 /* for tm_gmtoff field in struct tm */

#define EXIT_CIRCUMPOLAR 2

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <float.h>
#include <math.h>
#include <libgen.h>
#include <time.h>
#include <libnova/libnova.h>

#include "../config.h"
#include "objects.h"
#include "helpers.h"
#include "formatter.h"
#include "geonames.h"

enum moment {
	MOMENT_NOW,
	MOMENT_RISE,
	MOMENT_SET,
	MOMENT_TRANSIT
};

static struct option long_options[] = {
	{"object",	required_argument, 0, 'p'},
	{"horizon",	required_argument, 0, 'H'},
	{"time",	required_argument, 0, 't'},
	{"moment",	required_argument, 0, 'm'},
	{"next",	no_argument,	   0, 'n'},
	{"format",	required_argument, 0, 'f'},
	{"lat",		required_argument, 0, 'a'},
	{"lon",		required_argument, 0, 'o'},
#ifdef GEONAMES_SUPPORT
	{"query",	required_argument, 0, 'q'},
#endif
	{"timezone",	required_argument, 0, 'z'},
	{"universal",	no_argument,	   0, 'u'},
	{"help",	no_argument,	   0, 'h'},
	{"version",	no_argument,	   0, 'v'},
	{0}
};

static const char *long_options_descs[] = {
	"calc for celestial object: sun, moon, mars, neptune,\n\t\t\t jupiter, mercury, uranus, saturn, venus or pluto",
	"calc rise/set time with twilight: nautic, civil or astronomical",
	"calc at given time: YYYY-MM-DD [HH:MM:SS]",
	"calc position at moment of: rise, set, transit",
	"use rise, set, transit time of tomorrow",
	"output format: see strftime (3) and calcelestial (1) for more details",
	"geographical latitude of observer: -90° to 90°",
	"geographical longitude of oberserver: -180° to 180°",
#ifdef GEONAMES_SUPPORT
	"query geonames.org for geographical coordinates",
#endif
	"override system timezone",
	"use universial time for parsing and formatting",
	"show this help",
	"show version"
};

void version() {
	printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
	printf("libnova %s\n", LIBNOVA_VERSION);
}

void usage() {
	printf("Usage:\n  %s [options]\n\n", PACKAGE_NAME);
	printf("Options:\n");

	struct option *op = long_options;
	const char **desc = long_options_descs;
	while (op->name && desc) {
		printf("  -%c, --%s%s%s\n", op->val, op->name, (strlen(op->name) <= 7) ? "\t\t" : "\t", *desc);
		op++;
		desc++;
	}

	printf("\nA combination of --lat & --lon or --query is required.\n");
	printf("Please report bugs to: %s\n", PACKAGE_BUGREPORT);
}

int main(int argc, char *argv[]) {
	/* default options */
	double horizon = LN_SOLAR_STANDART_HORIZON; /* 50 Bogenminuten; no twilight, normal sunset/rise */
	int tz = INT_MAX;
//	char *format = "time: %Y-%m-%d %H:%M:%S az: §a (§s) alt: §h";
	char *format = "%H:%M";
	char *query = NULL;
	bool error = false;
	bool utc = false;
	bool next = false;

	time_t t;
	double jd;
	struct tm *date = NULL;

	enum moment moment = MOMENT_NOW;
	enum object obj = OBJECT_INVALID;

	struct ln_lnlat_posn obs = { DBL_MAX, DBL_MAX };
	struct object_details result;

	/* parse planet/obj */
	obj = object_from_name(basename(argv[0]));

	/* parse command line arguments */
	while (1) {
		int c = getopt_long(argc, argv, "+hvnut:d:f:a:o:q:z:p:m:H:", long_options, NULL);

		/* detect the end of the options. */
		if (c == -1) break;

		switch (c) {
			case 'H':
				if (strcmp(optarg, "civil") == 0) {
					horizon = LN_SOLAR_CIVIL_HORIZON;
				}
				else if (strcmp(optarg, "nautic") == 0) {
					horizon = LN_SOLAR_NAUTIC_HORIZON;
				}
				else if (strcmp(optarg, "astronomical") == 0) {
					horizon = LN_SOLAR_ASTRONOMICAL_HORIZON;
				}
				else {
					char *endptr;
					horizon = strtod(optarg, &endptr);

					if (endptr == optarg) {
						fprintf(stderr, "invalid twilight: %s\n", optarg);
						error = true;
					}
				}
				break;

			case 't':
				date = malloc(sizeof(struct tm));
				date->tm_isdst = -1; /* update dst */
				if (strptime(optarg, "%Y-%m-%d %H:%M:%S", date)) { }
				else if (strptime(optarg, "%Y-%m-%d", date)) { }
				else {
					free(date);
					fprintf(stderr, "invalid date: %s\n", optarg);
					error = true;
				}
				break;

			case 'm':
				if (strcmp(optarg, "rise") == 0) moment = MOMENT_RISE;
				else if (strcmp(optarg, "set") == 0) moment = MOMENT_SET;
				else if (strcmp(optarg, "transit") == 0) moment = MOMENT_TRANSIT;
				else {
					fprintf(stderr, "invalid moment: %s\n", optarg);
					error = true;
				}
				break;

			case 'n':
				next = true;
				break;

			case 'f':
				format = strdup(optarg);
				break;

			case 'a':
				obs.lat = strtod(optarg, NULL);
				break;

			case 'o':
				obs.lng = strtod(optarg, NULL);
				break;
#ifdef GEONAMES_SUPPORT
			case 'q':
				query = strdup(optarg);
				break;
#endif

			case 'p':
				obj = object_from_name(optarg);
				break;

			case 'z':
				tz = atoi(optarg);
				break;

			case 'u':
				utc = true;
				tz = 0;
				break;

			case 'v':
				version();
				return 0;

			case 'h':
				usage();
				return 0;

			case '?':
			default:
				fprintf(stderr, "unrecognized option %s\n", optarg);
				error = true;
		}
	}

	/* validate obj */
	if (obj == OBJECT_INVALID) {
		fprintf(stderr, "invalid object, use --object\n");
		error = true;
	}

#ifdef GEONAMES_SUPPORT
	/* lookup place at http://geonames.org */
	if (query && geonames_lookup(query, &obs, NULL, 0) != 0) {
		fprintf(stderr, "failed to lookup location: %s\n", query);
		error = true;
	}
#endif

	/* validate observer coordinates */
	if (fabs(obs.lat) > 90) {
		fprintf(stderr, "invalid latitude, use --lat\n");
		error = true;
	}
	if (fabs(obs.lng) > 180) {
		fprintf(stderr, "invalid longitude, use --lon\n");
		error = true;
	}

	/* abort on errors */
	if (error) {
		printf("\n");
		usage();
		return -1;
	}

	/* calculate julian date */
	if (date) {
		t = (utc) ? mktimeutc(date) : mktime(date);
		free(date);
	}
	else
		t = time(NULL);

	jd = ln_get_julian_from_timet(&t);
	date = localtime(&t);

	result.tz = (tz == INT_MAX) ? date->tm_gmtoff / 3600 : tz;
	result.obs = obs;

#ifdef DEBUG
	printf("calculate for jd: %f\n", jd);
	printf("calculate for ts: %ld\n", t);
	printf("for position: N %f, E %f\n", obs.lat, obs.lng);
	printf("for object: %s\n", object_to_name(obj));
	printf("with horizon: %f\n", horizon);
	printf("with timezone: %d\n", result.tz);
#endif

	/* calc rst date */
rst:	if (object_rst(obj, jd - .5, horizon, &result.obs, &result.rst) == 1)  {
		if (moment != MOMENT_NOW) {
			fprintf(stderr, "object is circumpolar\n");
			return 2;
		}
	}
	else {
		switch (moment) {
			case MOMENT_NOW:	result.jd = jd; break;
			case MOMENT_RISE:	result.jd = result.rst.rise; break;
			case MOMENT_SET:	result.jd = result.rst.set; break;
			case MOMENT_TRANSIT:	result.jd = result.rst.transit; break;
		}

		if (next && result.jd < jd) {
			jd++;
			next = false;
			goto rst;
		}
	}

	/* calc position */
	object_pos(obj, result.jd, &result);

	/* format & output */
	format_result(format, &result);

	return 0;
}
