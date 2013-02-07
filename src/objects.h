/**
 * libnova bindings
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

#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include <libnova/libnova.h>

#include <stdbool.h>

#define AU_METERS 149597870.7

enum object {
	OBJECT_INVALID,
	OBJECT_SUN,
	OBJECT_MOON,
	OBJECT_MARS,
	OBJECT_NEPTUNE,
	OBJECT_JUPITER,
	OBJECT_MERCURY,
	OBJECT_URANUS,
	OBJECT_SATURN,
	OBJECT_VENUS,
	OBJECT_PLUTO
};

struct object_details {
	struct ln_rst_time rst;	/* rise/set/transit time in JD */

	struct ln_equ_posn equ;
	struct ln_hrz_posn hrz;
	double diameter;		/* in arc seconds */
	double distance;		/* in AU */
};

enum object object_from_name(const char *name, bool casesen);

void object_pos(enum object obj, double jd, struct ln_lnlat_posn *obs, struct object_details *details);
int object_rst(enum object obj, double jd, double horizon, struct ln_lnlat_posn *obs, struct ln_rst_time *rst);

void object_pos_sun(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_moon(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_mars(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_neptune(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_jupiter(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_mercury(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_uranus(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_saturn(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_venus(double jd, struct ln_lnlat_posn *obs, struct object_details *details);
void object_pos_pluto(double jd, struct ln_lnlat_posn *obs, struct object_details *details);

#endif /* _OBJECTS_H_ */
