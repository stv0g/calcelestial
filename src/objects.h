/**
 * libnova bindings
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

#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include <time.h>
#include <libnova/libnova.h>


struct object;

struct object_details {
	double jd;			/**< Julian date of observation */
	struct tm tm;			/**< Broken down representation of observation */
	
	double diameter;		/**< In arc seconds */
	double distance;		/**< In AU (astronomical unit) */

	struct ln_lnlat_posn obs;	/**< Observer position */
	struct ln_rst_time rst;		/**< Rise/set/transit time in JD */

	struct ln_equ_posn equ;
	struct ln_hrz_posn hrz;
	const char *azidir;		/**< Direction of azimuth - like N,S,W,E,NW,.. */
};

const struct object * object_lookup(const char *name);
const char * object_name(const struct object *o);

void object_pos(const struct object *o, double jd, struct object_details *details);
int object_rst(const struct object *o, double jd, double horizon, struct ln_lnlat_posn *obs, struct ln_rst_time *rst);

#endif /* _OBJECTS_H_ */
