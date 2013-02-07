/**
 * Helper functions
 *
 * @copyright	2012 Steffen Vogel
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <post@steffenvogel.de>
 * @link	http://www.steffenvogel.de/2012/03/14/cron-jobs-fur-sonnenauf-untergang/
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <libnova/libnova.h>

#include "helpers.h"

char * strfjddur(char *s, size_t max, const char *format, double jd) {
	struct tm tmd;
	struct ln_date lnd;

	char *local_format = strdup(format);

	ln_get_date(jd + 0.5, &lnd);

	if (strstr(format, "%s") != NULL) {
		char timestamp_str[16];
		int seconds = round(jd * 86400);
		snprintf(timestamp_str, sizeof(timestamp_str), "%lu", seconds);
		format = strreplace(local_format, "%s", timestamp_str);
	}

	tmd.tm_year = -1900;
	tmd.tm_mon = -1;
	tmd.tm_mday = 0;
	tmd.tm_hour = lnd.hours;
	tmd.tm_min = lnd.minutes;
	tmd.tm_sec = lnd.seconds;

	strftime(s, max, local_format, &tmd);
	free(local_format);
	return s;
}

char * strfjd(char *s, size_t max, const char *format, double jd) {
	struct tm tmd;
	struct ln_date lnd;

//	time_t t;
//	ln_get_timet_from_julian(jd, &t);
//	strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", gmtime(&t));
//	ln_get_date(jd - timezone / 86400.0, &lnd);

	ln_get_date(jd, &lnd);

	tmd.tm_year = lnd.years - 1900;
	tmd.tm_mon = lnd.months - 1;
	tmd.tm_mday = lnd.days;
	tmd.tm_hour = lnd.hours;
	tmd.tm_min = lnd.minutes;
	tmd.tm_sec = lnd.seconds;

	strftime(s, max, format, &tmd);
	return s;
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
