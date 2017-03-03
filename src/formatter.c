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
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "objects.h"
#include "formatter.h"

#define PRECISION "3"

struct specifiers {
	const char *token;
	const char *desc;
	size_t offset; // offset in struct object_details
	enum { DOUBLE, STRING, INTEGER } format;
};

static struct specifiers specifiers[] = {
	{ "§J", "Julian date of observation",				offsetof(struct object_details, jd),		DOUBLE },
	{ "§d", "Diameter in arc seconds",				offsetof(struct object_details, diameter),	DOUBLE },
	{ "§e", "Distance to object in astronomical unit",		offsetof(struct object_details, distance),	DOUBLE },
	{ "§r", "Equatorial Coordinates: Right Ascension in degrees",	offsetof(struct object_details, equ.ra),	DOUBLE },
	{ "§d", "Equatorial Coordinates: Declincation in degrees",	offsetof(struct object_details, equ.dec),	DOUBLE },
	{ "§a", "Horizontal Coordinates: Azimuth in degrees",		offsetof(struct object_details, hrz.az),	DOUBLE },
	{ "§h", "Horizontal Coordinates: Altitude in degrees",		offsetof(struct object_details, hrz.alt),	DOUBLE },
	{ "§A", "Latitude in degrees",					offsetof(struct object_details, obs.lat),	DOUBLE },
	{ "§O", "Longitude in degrees",					offsetof(struct object_details, obs.lng),	DOUBLE },
	{ "§s", "Azimuth direction (N, E, S, W, NE, ...)",		offsetof(struct object_details, azidir),	STRING },
	{ NULL }
};

/** Replace parts of the string with a possibily long replacement */
char * strrepl(const char *subject, const char *search, const char *replace)
{
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

void print_format_tokens()
{
	int i;
	
	printf("The following special tokens are supported in the --format parameter:\n\n");
	
	for (i = 0; specifiers[i].token; i++)
		printf("  %s\t%s (%d)\n", specifiers[i].token, specifiers[i].desc, specifiers[i].offset);
	
	printf("\n");
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

	for (i = 0; specifiers[i].token; i++) {
		if (strstr(local_format, specifiers[i].token) != NULL) {
			void *ptr = (char *) result + specifiers[i].offset;
			
			switch (specifiers[i].format) {
				case DOUBLE: snprintf(buffer, sizeof(buffer), "%." PRECISION "f", * (double *) ptr); break;
				case STRING: snprintf(buffer, sizeof(buffer), "%s",                 (char *)   ptr); break;
				case INTEGER: snprintf(buffer, sizeof(buffer), "%d",              * (int *)    ptr); break;
			}

			local_format = strrepl(local_format, specifiers[i].token, buffer);
		}
	}

	strftime(buffer, sizeof(buffer), local_format, &result->tm);
	printf("%s\n", buffer);

	free(local_format);
}
