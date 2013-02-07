/**
 * Header file for Geonames.org lookup
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

#ifndef _GEONAMES_H_
#define _GEONAMES_H_

#include <json/json.h>

#define GEONAMES_CACHE_SUPPORT 1
#define GEONAMES_CACHE_FILE ".geonames.cache" /* in users home dir */

struct coords {
	double lng;
	double lat;
};

int geonames_lookup(const char *place, struct coords *coords, char *name, int n);
int geonames_cache_lookup(const char *place, struct coords *result, char *name, int n);
int geonames_cache_store(const char *place, struct coords *result, char *name, int n);
int geonames_parse(struct json_object *jobj, struct coords *result, char *name, int n);

#endif /* _GEONAMES_H_ */
