/**
 * Calculation routines
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

#include <math.h>
#include <stdio.h>

#include "../config.h"
#include "sun.h"

double deg2rad(double deg) {
	return M_PI * deg / 180.0;
}

double rad2deg(double rad) {
	return 180 * rad / M_PI;
}

int julian_date(struct tm date) {
	int year = date.tm_year + 1900;
	int month = date.tm_mon + 1;
	int day = date.tm_mday;

	int a = (14 - month) / 12;
	int y = year + 4800 - a;
	int m = month + 12 * a - 3;
	int jdn = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;

	return jdn;
}

double in_pi(double x) {
	x -= floor(x / M_2PI) * M_2PI;

	return (x < 0) ? x + M_2PI : x;
}

double sun_eps(double days) {
	double poly;

	poly = -46.8150 * days - 0.00059 * powf(days, 2) + 0.001813 * powf(days, 3);
	poly /= 3600.0;

	return deg2rad(23.43929111 + poly);
}

double sun_eof(double days, double ra) {
	double ra_mean = 18.71506921 + 2400.0513369 * days + (2.5862e-5 - 1.72e-9 * days) * powf(days, 2);
	ra_mean = 24.0 * in_pi(M_2PI * ra_mean / 24.0) / M_2PI;
	double ra_diff = ra_mean - ra;

	if (ra_diff < -12) ra_diff += 24;
	if (ra_diff > 12) ra_diff -= 24;

	ra_diff *= 1.0027379;

#ifdef DEBUG
	printf("ra_mean = %.20f\n", ra_mean);
	printf("ra_diff = %.20f\n", ra_diff);
#endif

	return ra_diff;
}

struct sun_coords sun_pos(double days) {
	double m  = in_pi(M_2PI * (0.993133 + 99.997361 * days)); 
	double l  = in_pi(M_2PI * (0.7859453 + m / M_2PI  + (6893 * sin(m) + 72.0 * sin(2 * m) + 6191.2 * days) / 1296.0e3));
	double e = sun_eps(days);
	double ra =  atan(tan(l)*cos(e));
	double dk = asin(sin(e) * sin(l));

	if (ra < 0) ra += M_PI;
	if (l > M_PI) ra += M_PI;

	ra *= 12 / M_PI;

#ifdef DEBUG
	printf("m = %.20f\n", m);
	printf("l = %.20f\n", l);
	printf("e = %.20f\n", e);
	printf("dk = %.20f\n", dk);
	printf("ra = %.20f\n", ra);
#endif

	struct sun_coords spos = { dk, ra };
	return spos;
}

struct tm sun(struct coords pos, struct tm date, enum mode mode, double twilight, int timezone) {
	/* calculate sun position */
	double jd = julian_date(date);
	double days = (jd - 2451545) / 36525;
	struct sun_coords spos = sun_pos(days);

	/* use equation of time */
	double h = deg2rad(twilight);
	double time = sun_eof(days, spos.ra);
	double time_diff = 12.0 * acos((sin(h) - sin(deg2rad(pos.lat)) * sin(spos.dk)) / (cos(deg2rad(pos.lat)) * cos(spos.dk))) / M_PI;

#ifdef DEBUG
	printf("jd = %.20f\n", jd);
	printf("days = %.20f\n", days);
	printf("delta_days = %.20f\n", jd - 2451545);
	printf("h = %.20f\n", h);
	printf("time = %.20f\n", time);
	printf("time_diff = %.20f\n", time_diff);
#endif

	double local = 12 - time;
	double global = local - pos.lon / 15 + timezone;

	switch (mode) {
		case NOON: /* do nothing */ break;
		case RISE:	global -= time_diff; break;
		case SET:	global += time_diff; break;
		case DAYTIME:	global = 2 * time_diff; break;
		case NIGHTTIME:	global = 24 - 2 * time_diff; break;
	}


	if (global < 0) {
		global += 24;
	}
	else if (global >= 24) {
		global -= 24;
	}

#ifdef DEBUG
	printf("local = %f\n", local);
	printf("global = %f\n", global);
#endif

	date.tm_hour = (int) global;
	date.tm_min = 60 * (global - date.tm_hour) + 0.5; /* math rounding */
	date.tm_sec = 0; /* below our accuracy */
	//date.tm_sec = 3600 * (global - date.tm_hour - date.tm_min / 60.0);

	return date;
}
