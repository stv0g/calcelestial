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

#include <string.h>

#include "objects.h"

void object_pos(enum object obj, double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	switch (obj) {
		case OBJECT_SUN:	return object_pos_sun(jd, obs, details);
		case OBJECT_MOON:	return object_pos_moon(jd, obs, details);
		case OBJECT_MARS:	return object_pos_mars(jd, obs, details);
		case OBJECT_NEPTUNE:	return object_pos_neptune(jd, obs, details);
		case OBJECT_JUPITER:	return object_pos_jupiter(jd, obs, details);
		case OBJECT_MERCURY:	return object_pos_mercury(jd, obs, details);
		case OBJECT_URANUS:	return object_pos_uranus(jd, obs, details);
		case OBJECT_SATURN:	return object_pos_saturn(jd, obs, details);
		case OBJECT_VENUS:	return object_pos_venus(jd, obs, details);
		case OBJECT_PLUTO:	return object_pos_pluto(jd, obs, details);
	}
}

int object_rst(enum object obj, double jd, double horizon, struct ln_lnlat_posn *obs, struct ln_rst_time *rst) {
	switch (obj) {
		case OBJECT_SUN:	return ln_get_solar_rst_horizon(jd, obs, horizon, rst);
		case OBJECT_MOON:	return ln_get_lunar_rst(jd, obs, rst);
		case OBJECT_MARS:	return ln_get_mars_rst(jd, obs, rst);
		case OBJECT_NEPTUNE:	return ln_get_neptune_rst(jd, obs, rst);
		case OBJECT_JUPITER:	return ln_get_jupiter_rst(jd, obs, rst);
		case OBJECT_MERCURY:	return ln_get_mercury_rst(jd, obs, rst);
		case OBJECT_URANUS:	return ln_get_uranus_rst(jd, obs, rst);
		case OBJECT_SATURN:	return ln_get_saturn_rst(jd, obs, rst);
		case OBJECT_VENUS:	return ln_get_venus_rst(jd, obs, rst);
		case OBJECT_PLUTO:	return ln_get_pluto_rst(jd, obs, rst);
	}
}

enum object object_from_name(const char *name, bool casesen) {
	// TODO case sensivity

	if (strcmp(name, "sun") == 0)		return OBJECT_SUN;
	else if (strcmp(name, "moon") == 0)	return OBJECT_MOON;
	else if (strcmp(name, "mars") == 0)	return OBJECT_MARS;
	else if (strcmp(name, "neptune") == 0)	return OBJECT_NEPTUNE;
	else if (strcmp(name, "jupiter") == 0)	return OBJECT_JUPITER;
	else if (strcmp(name, "mercury") == 0)	return OBJECT_MERCURY;
	else if (strcmp(name, "uranus") == 0)	return OBJECT_URANUS;
	else if (strcmp(name, "saturn") == 0)	return OBJECT_SATURN;
	else if (strcmp(name, "venus") == 0)	return OBJECT_VENUS;
	else if (strcmp(name, "pluto") == 0)	return OBJECT_PLUTO;
	else return OBJECT_INVALID;
}

void object_pos_sun(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_solar_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_earth_solar_dist(jd);
	details->diameter = ln_get_solar_sdiam(jd);
}

void object_pos_moon(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_lunar_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_lunar_earth_dist(jd) / AU_METERS;
	details->diameter = ln_get_lunar_sdiam(jd);
}

void object_pos_mars(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_mars_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_mars_earth_dist(jd);
	details->diameter = ln_get_mars_sdiam(jd);
}

void object_pos_neptune(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_neptune_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_neptune_earth_dist(jd);
	details->diameter = ln_get_neptune_sdiam(jd);
}

void object_pos_jupiter(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_jupiter_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_jupiter_earth_dist(jd);
	details->diameter = ln_get_jupiter_equ_sdiam(jd);
}

void object_pos_mercury(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_mercury_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_mercury_earth_dist(jd);
	details->diameter = ln_get_mercury_sdiam(jd);
}

void object_pos_uranus(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_uranus_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_uranus_earth_dist(jd);
	details->diameter = ln_get_uranus_sdiam(jd);
}

void object_pos_saturn(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_saturn_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_saturn_earth_dist(jd);
	details->diameter = ln_get_saturn_equ_sdiam(jd);
}

void object_pos_venus(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_venus_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_venus_earth_dist(jd);
	details->diameter = ln_get_venus_sdiam(jd);
}

void object_pos_pluto(double jd, struct ln_lnlat_posn *obs, struct object_details *details) {
	ln_get_pluto_equ_coords(jd, &details->equ);
	ln_get_hrz_from_equ(&details->equ, obs, jd, &details->hrz);

	details->distance = ln_get_pluto_earth_dist(jd);
	details->diameter = ln_get_pluto_sdiam(jd);
}
