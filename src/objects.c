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

#include <string.h>

#include "objects.h"

static struct object {
	const char *name;
	int (*rst)(double JD, struct ln_lnlat_posn *observer, struct ln_rst_time *rst);
	void (*equ_coords)(double JD, struct ln_equ_posn *position);
	double (*earth_dist)(double JD);
	double (*sdiam)(double JD);
} objects[] = {
	{ "sun",     0 /* special case */, ln_get_solar_equ_coords,   ln_get_earth_solar_dist,   ln_get_solar_sdiam       },
	{ "moon",    ln_get_lunar_rst,     ln_get_lunar_equ_coords,   ln_get_lunar_earth_dist,   ln_get_lunar_sdiam       },
	{ "mars",    ln_get_mars_rst,      ln_get_mars_equ_coords,    ln_get_mars_earth_dist,    ln_get_mars_sdiam        },
	{ "neptune", ln_get_neptune_rst,   ln_get_neptune_equ_coords, ln_get_neptune_earth_dist, ln_get_neptune_sdiam     },
	{ "jupiter", ln_get_jupiter_rst,   ln_get_jupiter_equ_coords, ln_get_jupiter_earth_dist, ln_get_jupiter_equ_sdiam },
	{ "mercury", ln_get_mercury_rst,   ln_get_mercury_equ_coords, ln_get_mercury_earth_dist, ln_get_mercury_sdiam     },
	{ "uranus",  ln_get_uranus_rst,    ln_get_uranus_equ_coords,  ln_get_uranus_earth_dist,  ln_get_uranus_sdiam      },
	{ "saturn",  ln_get_saturn_rst,    ln_get_saturn_equ_coords,  ln_get_saturn_earth_dist,  ln_get_saturn_equ_sdiam  },
	{ "venus",   ln_get_venus_rst,     ln_get_venus_equ_coords,   ln_get_venus_earth_dist,   ln_get_venus_sdiam       },
	{ "pluto",   ln_get_pluto_rst,     ln_get_pluto_equ_coords,   ln_get_pluto_earth_dist,   ln_get_pluto_sdiam       }
};

const struct object * object_lookup(const char *name)
{
	int c;
	for (c = 0; c <= sizeof(objects) / sizeof(objects[0]); c++) {
		if (strcmp(objects[c].name, name) == 0)
			return &objects[c];
	}

	return NULL;
}

const char * object_name(const struct object *o)
{
	return o->name;
}

void object_pos(const struct object *o, double jd, struct object_details *details)
{
	o->equ_coords(jd, &details->equ);

	details->distance = o->earth_dist(jd);
	details->diameter = o->sdiam(jd);
}

int object_rst(const struct object *o, double jd, double horizon, struct ln_lnlat_posn *obs, struct ln_rst_time *rst)
{
	if (o->sdiam == ln_get_solar_sdiam)
		return ln_get_solar_rst_horizon(jd, obs, horizon, rst); /* special case */
	else
		return o->rst(jd, obs, rst);
}