/**
 * Header file for Geonames.org lookup
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

#ifndef _GEONAMES_H_
#define _GEONAMES_H_

#include <json-c/json.h>

/* Forward declaration */
struct ln_lnlat_posn;

#define GEONAMES_CACHE_SUPPORT 0

#define GEONAMES_CACHE_FILE ".geonames.db" /* in users home dir */

int geonames_lookup_latlng(const char *place, struct ln_lnlat_posn *coords, char *name, size_t namelen);
int geonames_lookup_tz(struct ln_lnlat_posn coords, int *gmt_offset, char *tzid, size_t tzidlen);

#endif /* _GEONAMES_H_ */
