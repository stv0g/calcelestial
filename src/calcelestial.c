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
#include "formatter.h"
#include "geonames.h"

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
	{"local",	no_argument,	   0, 'l'},
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
	"calc at given time: YYYY-MM-DD[_HH:MM:SS]",
	"calc position at moment of: rise, set, transit",
	"use rise, set, transit time of tomorrow",
	"output format: see strftime (3) and calcelestial (1) for more details",
	"geographical latitude of observer: -90° to 90°",
	"geographical longitude of oberserver: -180° to 180°",
#ifdef GEONAMES_SUPPORT
	"query coordinates using the geonames.org geolocation service",
	"query local timezone using the geonames.org geolocation service",
#endif
	"override system timezone (TZ environment variable)",
	"use universial time for parsing and formatting",
	"show usage help",
	"show version"
};

void version()
{
	printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
	printf("libnova %s\n", LIBNOVA_VERSION);
}

void usage()
{
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

void usage_error(const char *err)
{
	fprintf(stderr, "Error: %s\n\n", err);
	usage();
	exit(-1);
}

int main(int argc, char *argv[])
{
	int ret;
	time_t t;
	double jd;
	struct tm tm;
	const struct object *obj;

	/* Default options */
	double horizon = LN_SOLAR_STANDART_HORIZON; /* 50 Bogenminuten; no twilight, normal sunset/rise */
	int tz = INT_MAX;

	char *obj_str = basename(argv[0]);
	char *format = "time: %Y-%m-%d %H:%M:%S (%Z) az: §a (§s) alt: §h";
	char tzid[32];
	char *query = NULL;

	bool horizon_set = false;
	bool next = false;
	bool local_tz = false;
	
	time(&t);
	localtime_r(&t, &tm);

	enum {
		MOMENT_NOW,
		MOMENT_RISE,
		MOMENT_SET,
		MOMENT_TRANSIT
	} moment = MOMENT_NOW;

	struct ln_lnlat_posn obs = { DBL_MAX, DBL_MAX };
	struct object_details result;

	/* set tzid as empty (without repointing the buffer) */
	strcpy(tzid, "");
	/* parse command line arguments */
	while (1) {
		int c = getopt_long(argc, argv, "+hvnult:d:f:a:o:q:z:p:m:H:", long_options, NULL);

		/* detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
			case 'H':
				if      (strcmp(optarg, "civil") == 0)
					horizon = LN_SOLAR_CIVIL_HORIZON;
				else if (strcmp(optarg, "nautic") == 0)
					horizon = LN_SOLAR_NAUTIC_HORIZON;
				else if (strcmp(optarg, "astronomical") == 0)
					horizon = LN_SOLAR_ASTRONOMICAL_HORIZON;
				else {
					char *endptr;
					horizon = strtod(optarg, &endptr);

					if (endptr == optarg)
						usage_error("invalid horizon / twilight parameter");
				}
				
				horizon_set = true;
				break;

			case 't':
				tm.tm_isdst = -1; /* update dst */
				if (strchr(optarg, '_')) {
					if (!strptime(optarg, "%Y-%m-%d_%H:%M:%S", &tm))
						usage_error("invalid time/date parameter");
				}
				else {
					if (!strptime(optarg, "%Y-%m-%d", &tm))
						usage_error("invalid time/date parameter");
				}
				break;

			case 'm':
				if      (strcmp(optarg, "rise") == 0)
					moment = MOMENT_RISE;
				else if (strcmp(optarg, "set") == 0)
					moment = MOMENT_SET;
				else if (strcmp(optarg, "transit") == 0)
					moment = MOMENT_TRANSIT;
				else
					usage_error("invalid moment");
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

			case 'l':
				local_tz = true;
				break;
#endif
			case 'p':
				obj_str = optarg;
				break;

			case 'z':
				strncpy(tzid, optarg, sizeof(tzid));
				break;

			case 'u':
				strncpy(tzid, "UTC", sizeof(tzid));
				break;

			case 'v':
				version();
				return 0;

			case 'h':
				usage();
				return 0;

			case '?':
			default:
				usage_error("unrecognized option");
		}
	}
	
	/* Parse planet/obj */
	obj = object_lookup(obj_str);
	if (!obj)
		usage_error("invalid or missing object, use --object");

#ifdef GEONAMES_SUPPORT
	/* Lookup place at http://geonames.org */
	if (query) {
		ret =  geonames_lookup_latlng(query, &obs, NULL, 0);
		if (ret)
			usage_error("failed to lookup location");
	}
	
	if (local_tz) {
		int gmt_offset;
		ret =  geonames_lookup_tz(obs, &gmt_offset, tzid, sizeof(tzid));
		if (ret)
			usage_error("failed to lookup location");
	}
#endif

	if(strlen(tzid) > 0)	/* set TZ variable only when we have a value - otherwise rely on /etc/localtime or whatever other system fallbacks */
		setenv("TZ", tzid, 1);
	tzset();

	/* Validate observer coordinates */
	if (fabs(obs.lat) > 90)
		usage_error("invalid latitude, use --lat");
	if (fabs(obs.lng) > 180)
		usage_error("invalid longitude, use --lon");
	
	if (horizon_set && strcmp(object_name(obj), "sun"))
		usage_error("the twilight parameter can only be used for the sun");

	/* Calculate julian date */
	t = mktime(&tm);
	jd = ln_get_julian_from_timet(&t);

	result.obs = obs;

#ifdef DEBUG
	printf("Debug: calculate for jd: %f\n", jd);
	printf("Debug: calculate for ts: %ld\n", t);
	printf("Debug: for position: N %f, E %f\n", obs.lat, obs.lng);
	printf("Debug: for object: %s\n", object_name(obj));
	printf("Debug: with horizon: %f\n", horizon);
	printf("Debug: with timezone: %s\n", tzid);
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
	
	ln_get_timet_from_julian(result.jd, &t);
	localtime_r(&t, &result.tm);

	object_pos(obj, jd, &result);

	format_result(format, &result);

	return 0;
}
