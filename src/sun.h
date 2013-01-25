/**
 * Header file for calculation
 *
 * @copyright	2012 Steffen Vogel
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <post@steffenvogel.de>
 * @link	http://www.steffenvogel.de/2012/03/14/cron-jobs-fur-sonnenauf-untergang/
 */
/*
 * This file is part of sun
 *
 * sun is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * sun is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sun. If not, see <http://www.gnu.org/licenses/>.
 */


#include <time.h>

#ifdef GEONAMES_SUPPORT
#include "geonames.h"
#else
struct coords { double lat, lon; };
#endif

struct sun_coords { double dk, ra; };

#define M_2PI (M_PI * 2)

enum mode { INVALID, RISE, SET, NOON, DAYTIME, NIGHTTIME };

/* helper function */
void usage();
double deg2rad(double deg);
double rad2deg(double rad);
double in_pi(double x);
int julian_date(struct tm date);

/* calculation functions */
struct tm sun(struct coords pos, struct tm date, enum mode mode, double twilight, int timezone);
struct sun_coords sun_pos(double days);
double sun_eof(double days, double ra);
double sun_eps(double days);

