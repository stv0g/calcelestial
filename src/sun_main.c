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

#define _XOPEN_SOURCE 700
#define EXIT_CIRCUMPOLAR 2

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <float.h>
#include <math.h>

#include <time.h>
#include <sys/time.h>

#include <libgen.h>

#include <libnova/libnova.h>
#include <libnova/utility.h>

#include "../config.h"
#include "objects.h"

enum mode {
	MODE_NOW,
	MODE_RISE,
	MODE_SET,
	MODE_TRANSIT
};

extern long timezone;

static struct option long_options[] = {
	{"object",	required_argument, 0, 'p'},
	{"horizon",	required_argument, 0, 'H'},
	{"time",	required_argument, 0, 't'},
	{"moment",	required_argument, 0, 'm'},
	{"format",	required_argument, 0, 'f'},
	{"lat",		required_argument, 0, 'a'},
	{"lon",		required_argument, 0, 'o'},
#ifdef GEONAMES_SUPPORT
	{"query",	required_argument, 0, 'q'},
#endif
	{"timezone",	required_argument, 0, 'z'},
	{"help",	no_argument,	   0, 'h'},
	{"version",	no_argument,	   0, 'v'},
	{0}
};

static char *long_options_descs[] = {
	"calculate for given object/planet",
	"calculate rise/set with given twilight (nautic|civil|astronomical)",
	"calculate with given time (eg. 2011-12-25)",
	"use rise/set/transit time for position calculation",
	"output format (see strftime (3))",
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
	printf("libnova %s\n", LIBNOVA_VERSION);
}

void usage() {
	printf("Usage:\n  %s [options]\n\n", PACKAGE_NAME);
	printf("Options:\n");

	struct option *op = long_options;
	char **desc = long_options_descs;
	while (op->name && desc) {
		printf("  -%c, --%s%s%s\n", op->val, op->name, (strlen(op->name) <= 7) ? "\t\t" : "\t", *desc);
		op++;
		desc++;
	}

	printf("\nA combination of --lat, --lon or --query is required.\n");
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
	double horizon = LN_SOLAR_STANDART_HORIZON; /* 50 Bogenminuten; no twilight, normal sunset/rise */
	char *format = "%H:%M:%S";
	char *query = NULL;
	bool error = false;

	enum mode mode = MODE_NOW;
	enum object obj = OBJECT_INVALID;

	double jd;
	struct tm date = { 0 };
	struct ln_rst_time rst;
	struct ln_lnlat_posn obs = { DBL_MAX, DBL_MAX };

	tzset();

	/* default time: now */
	jd = ln_get_julian_from_sys();

	/* parse planet/obj */
	obj = object_from_name(basename(argv[0]), false);

	/* parse command line arguments */
	while (1) {
		int optidx;
		int c = getopt_long(argc, argv, "+hvt:d:f:a:o:q:z:p:", long_options, &optidx);

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
					fprintf(stderr, "invalid twilight: %s\n", optarg);
					error = true;
				}
				break;

			case 't':
				if (strptime(optarg, "%Y-%m-%d", &date) == NULL) {
					fprintf(stderr, "invalid date: %s\n", optarg);
					error = true;
				}
				else {
					time_t t = mktime(&date);
					jd = ln_get_julian_from_timet(&t);

#ifdef DEBUG
				        char date_str[64];
				        strftime(date_str, 64, "%Y-%m-%d", &date);
				        printf("parsed date: %s\n", date_str);
#endif

				}
				break;

			case 'm':
				if (strcmp(optarg, "now") == 0) mode = MODE_NOW;
				else if (strcmp(optarg, "rise") == 0) mode = MODE_RISE;
				else if (strcmp(optarg, "set") == 0) mode = MODE_SET;
				else if (strcmp(optarg, "transit") == 0) mode = MODE_TRANSIT;
				else {
					fprintf(stderr, "invalid moment: %s\n", optarg);
					error = true;
				}
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
				obj = object_from_name(optarg, false);
				break;

			case 'z':
				timezone = -3600 * atoi(optarg);
				break;

			case 'v':
				version();
				return EXIT_SUCCESS;

			case 'h':
				usage();
				return EXIT_SUCCESS;

			case '?':
			default:
				fprintf(stderr, "unrecognized option %s\n", optarg);
				error = true;
		}
	}

	/* validate obj */
	if (obj == OBJECT_INVALID) {
		fprintf(stderr, "invalid object\n");
		error = true;
	}

#ifdef GEONAMES_SUPPORT
	/* lookup place at http://geonames.org */
	if (query && geonames_lookup(query, (struct pos *) &obs, NULL, 0) != 0) {
		fprintf(stderr, "failed to lookup location: %s\n", query);
		error = true;
	}
#endif

	/* validate observer coordinates */
	if (fabs(obs.lat) > 90) {
		fprintf(stderr, "invalid latitude\n");
		error = true;
	}
	if (fabs(obs.lng) > 180) {
		fprintf(stderr, "invalid longitude\n");
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
	time_t t;
	ln_get_timet_from_julian(jd, &t);

	strftime(date_str, 64, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	printf("calculate for: %s\n", date_str);
	printf("calculate for jd: %f\n", jd);
	printf("for position: %f, %f\n", obs.lat, obs.lng);
	printf("for object: %d\n", obj);
	printf("with horizon: %f\n", horizon);
	printf("with timezone: UTC +%dh\n", timezone / -3600);
#endif

	char result_str[64];
	struct tm result_date;
	struct ln_date result_ln;

	if (object_rst(obj, jd, horizon, &obs, &rst) == 1)  {

                fprintf(stderr, "object is circumpolar\n");
		return EXIT_CIRCUMPOLAR;
	}

	switch (mode) {
		case MODE_RISE:		jd = rst.rise; break;
		case MODE_SET:		jd = rst.set; break;
		case MODE_TRANSIT:	jd = rst.transit; break;
		case MODE_DAYTIME:	jd = rst.set - rst.rise; break;
		case MODE_NIGHTTIME:	jd = rst.set - rst.rise; break;
		case MODE_INVALID:	break;
	}

	if (mode == MODE_DAYTIME || mode == MODE_NIGHTTIME) {
		ln_get_date(jd - 0.5, &result_ln);

		if (strstr(format, "%s") != NULL) {
			char timestamp_str[16];
			int seconds = round(jd * 86400);
			snprintf(timestamp_str, sizeof(timestamp_str), "%lu", seconds);
			format = strreplace(format, "%s", timestamp_str);
		}

		result_date.tm_year = -1900;
		result_date.tm_mon = -1;
		result_date.tm_mday = 0;
	}
	else {
		// calculate position
		struct ln_equ_posn result_equ;
		struct ln_hrz_posn result_hrz;
		double result_dist;
		double result_diam;

		object_pos(obj, jd, &obs, &result_equ, &result_hrz, &result_diam, &result_dist);

		struct ln_hms ra;
		ln_deg_to_hms(result_equ.ra, &ra);

		double az = result_hrz.az + 180;
		az -= (int) (az / 360) * 360;

		printf("diam = %f\n", result_diam);
		printf("dist au = %f\n", result_dist);
		printf("dist km = %f\n", AU_METERS * result_dist);
		printf("az = %s\n",  ln_get_humanr_location(az));
		printf("alt = %s\n", ln_get_humanr_location(result_hrz.alt));
		printf("ra = %dh%dm%fs\n", ra.hours, ra.minutes, ra.seconds);
		printf("dec = %s\n", ln_get_humanr_location(result_equ.dec));

		/*if (strstr(format, "%R") != NULL) {
			snprintf(timestamp_str, sizeof(timestamp_str), "%lu", seconds);
			format = strreplace(format, "%s", timestamp_str);
		}
		if (strstr(format, "%E") != NULL) {
			snprintf(timestamp_str, sizeof(timestamp_str), "%lu", seconds);
			format = strreplace(format, "%s", timestamp_str);
		}*/

		ln_get_date(jd - timezone / 86400.0, &result_ln);

		result_date.tm_year = result_ln.years - 1900;
		result_date.tm_mon = result_ln.months - 1;
		result_date.tm_mday = result_ln.days;
	}

	result_date.tm_hour = result_ln.hours;
	result_date.tm_min = result_ln.minutes;
	result_date.tm_sec = result_ln.seconds;

	strftime(result_str, 64, format, &result_date);
	printf("%s\n", result_str);

	return EXIT_SUCCESS;
}
