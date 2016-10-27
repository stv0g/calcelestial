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
#include "helpers.h"
#include "formatter.h"

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
		{"§t", &result->tz,		INTEGER},
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

			local_format = strreplace(local_format, specifiers[i].token, buffer);
		}
	}

	strfjd(buffer, sizeof(buffer), local_format, result->jd, result->tz);
	printf("%s\n", buffer);

	free(local_format);
}
