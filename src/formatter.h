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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with calcelestial. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FORMATTER_H_
#define _FORMATTER_H_

#include <libnova/libnova.h>

#define MAXLINELEN 512

/* Forward declaration */
struct object_details;

void format_result(const char *format, struct object_details *result);

size_t strlen_safe (const char * str, int maxlen);
int searchstr (char *str, char *srchstr, int start, int end, int case_sens);
int strrepl (const char *subject, const char *search, const char *replace, char *returnstr, long return_len);

/** Print a list of supported § tokens. */
void print_format_tokens();

#endif /* _FORMATTER_H_ */
