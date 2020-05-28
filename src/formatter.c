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

#define FALSE 0
#define TRUE 1


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

size_t strlen_safe (const char * str, int maxlen)
{
	 long long int i = 0;

	 while (1)
	 {
	 	if (str[i] != '\0')
		{
			i++;
		}
		else
		{
			return (i);
		}
		if (i > maxlen)
		{
			return (0);
		}
	}
}

int searchstr (char *str, char *srchstr, int start, int end, int case_sens)
{
	/* checked: ok! */

	int i, j = 0, pos = -1, str_len, srchstr_len;
	int ok = FALSE, check = TRUE;
	int new_end;
	char new_str, new_srchstr;

	str_len = strlen_safe ((const char *) str, MAXLINELEN);
	srchstr_len = strlen_safe ((const char *) srchstr, MAXLINELEN);

	if (start < 0 || start > str_len - 1)
	{
		i = 0;
	}
	else
	{
		i = start;
	}

	if (end == 0)
	{
		new_end = str_len - 1;
	}
	else
	{
		new_end = end;
	}

	while (! ok)
	{
		if (case_sens)
		{
			if (str[i] == srchstr[j])
			{
				pos = i;

				/* found start of searchstring, checking now */

				if (srchstr_len > 1)
				{
					for (j = j + 1; j <= srchstr_len - 1; j++)
					{
						if (i < new_end)
						{
							i++;
						}

						if (str[i] != srchstr[j]) check = FALSE;
					}
				}
				if (check)
				{
					ok = TRUE;
				}
				else
				{
					pos = -1;
				}
			}
			if (i < new_end)
			{
				i++;
			}
			else
			{
				ok = TRUE;
			}
		}
		else
		{
			new_str = str[i];
			new_srchstr = srchstr[j];

			if (str[i] >= 97 && str[i] <= 122)
			{
				new_str = str[i] - 32;
			}
			if (srchstr[j] >= 97 && srchstr[j] <= 122)
			{
				new_srchstr = srchstr[j] - 32;
			}

			if (new_str == new_srchstr)
			{
				pos = i;

				/* found start of searchstring, checking now */

				if (srchstr_len > 1)
				{
					for (j = j + 1; j <= srchstr_len - 1; j++)
					{
						if (i < new_end)
						{
							i++;
						}

						new_str = str[i];
						new_srchstr = srchstr[j];

						if (str[i] >= 97 && str[i] <= 122)
						{
							new_str = str[i] - 32;
						}
						if (srchstr[j] >= 97 && srchstr[j] <= 122)
						{
							new_srchstr = srchstr[j] - 32;
						}

						if (new_str != new_srchstr) check = FALSE;
					}
				}
				if (check)
				{
					ok = TRUE;
				}
				else
				{
					pos = -1;
				}
			}
			if (i < new_end)
			{
				i++;
			}
			else
			{
				ok = TRUE;
			}
		}
	}
	return (pos);
}

/** Replace parts of the string with a possibily long replacement */
int strrepl (const char *subject, const char *search, const char *replace, char *returnstr, long return_len)
{
	int subject_len = strlen (subject);
	int search_len = strlen (search);
	int replace_len = strlen (replace);
	int replace_pos;
	int i, j, sub;
	int return_pos = 0;

	// printf ("formatter.c: strrepl: subject: '%s'\nsearch: '%s'\nreplace: '%s'\n", subject, search, replace);

	replace_pos = searchstr ((char *) subject, (char *) search, 0, 0, 1);
	if (replace_pos == -1)
	{
		// search string not found
		strcpy (returnstr, subject);
		return (1);
	}

	if ((subject_len - search_len + replace_len) > return_len - 1)
	{
		// printf ("formatter.c: strrepl: ERROR return string overflow!\n");
		return (1);	// ERROR CODE
	}

	// printf ("formatter.c: strrepl: found searchstr on: %i\n", replace_pos);

	i = 0;
	while (i < subject_len)
	{
		if (i < replace_pos || i > replace_pos)
		{
			returnstr[return_pos] = *subject++;
			return_pos++;
		}
		if (i == replace_pos)
		{
			// insert replacestr to return string
			for (j = 0; j < replace_len; j++)
			{
				returnstr[return_pos] = *replace++;
				return_pos++;
			}
			for (sub = 0; sub < search_len; sub++)
			{
				subject++;
			}
		}
		i++;
	}
	returnstr[return_pos] = '\0';
	// printf ("strrepl: new: '%s'\n", returnstr);
	return (0);
}

void print_format_tokens()
{
	int i;

	printf("The following special tokens are supported in the --format parameter:\n\n");

	for (i = 0; specifiers[i].token; i++)
		printf("  %s\t%s\n", specifiers[i].token, specifiers[i].desc);

	printf("\n");
}

void format_result(const char *format, struct object_details *result)
{
	char buffer[128];
	char returnstr[256];
	char local_format[256];
	int i;

	struct ln_hms ra;

	strcpy (local_format, format);

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

			strrepl (local_format, (char *) specifiers[i].token, buffer, (char *) returnstr, 255);
			strcpy (local_format, returnstr);
		}
	}

	strftime(buffer, sizeof(buffer), returnstr, &result->tm);
	printf("%s\n", buffer);

	// free(local_format);
}
