/**
 * Formatter
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
 * GNU General Public License for more result.
 *
 * You should have received a copy of the GNU General Public License
 * along with calcelestial. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "objects.h"
#include "formatter.h"

#define PRECISION "3"

struct specifiers {
	const char *token;
	void *data;
	enum { DOUBLE, STRING, INTEGER } format;
};

/** Replace parts of the string with a possibily long replacement */
char * strrepl(const char *subject, const char *search, const char *replace) {
	int new_len = strlen(subject);
	int search_len = strlen(search);
	int replace_len = strlen(replace);
	char *tmp;

	for (tmp = strstr(subject, search); tmp != NULL; tmp = strstr(tmp + search_len, search))
		new_len += replace_len - search_len;

	const char *old = subject;
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

void format_result(const char *format, struct object_details *result)
{
	char buffer[128];
	char *local_format = strdup(format);
	int i;

	struct ln_hms ra;

	/* convert results */
	ln_deg_to_hms(result->equ.ra, &ra);
	ln_get_hrz_from_equ(&result->equ, &result->obs, result->jd, &result->hrz);

	result->azidir = ln_hrz_to_nswe(&result->hrz);

	result->hrz.az = ln_range_degrees(result->hrz.az + 180);
	result->hrz.alt = ln_range_degrees(result->hrz.alt);

	struct specifiers specifiers[] = {
		{"%J", &result->jd,		DOUBLE},
		{"§r", &result->equ.ra,		DOUBLE},
		{"§d", &result->equ.dec,	DOUBLE},
		{"§a", &result->hrz.az,		DOUBLE},
		{"§h", &result->hrz.alt,	DOUBLE},
		{"§d", &result->diameter,	DOUBLE},
		{"§e", &result->distance,	DOUBLE},
		{"§A", &result->obs.lat,	DOUBLE},
		{"§O", &result->obs.lng,	DOUBLE},
		{"§s", (void *) result->azidir, STRING},
		{"§§", "§",			STRING},
		{0}
	};

	for (i = 0; specifiers[i].token; i++) {
		if (strstr(local_format, specifiers[i].token) != NULL) {
			switch (specifiers[i].format) {
				case DOUBLE: snprintf(buffer, sizeof(buffer), "%." PRECISION "f", * (double *) specifiers[i].data); break;
				case STRING: snprintf(buffer, sizeof(buffer), "%s", (char *) specifiers[i].data); break;
				case INTEGER: snprintf(buffer, sizeof(buffer), "%d", * (int *) specifiers[i].data); break;
			}

			local_format = strrepl(local_format, specifiers[i].token, buffer);
		}
	}

	strftime(buffer, sizeof(buffer), local_format, &result->tm);
	printf("%s\n", buffer);

	free(local_format);
}
