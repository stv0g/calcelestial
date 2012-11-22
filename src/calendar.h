/**
 * Time and Calender calculus
 *
 * based on Practical astronomy with your calculator or spreadsheet
 *   from Peter Duffett-Smith, Jonathan Zwart. – 4th ed. (Cambridge University Press, 2011)
 *
 * @copyright	2012 Steffen Vogel
 * @license	http://www.gnu.org/licenses/gpl.txt GNU Public License
 * @author	Steffen Vogel <post@steffenvogel.de>
 * @author	Arnold Barmettler <barmettler@astronomie.info>
 * @link	http://www.steffenvogel.de
 */
/*
 * This file is part of libastro
 *
 * libastro is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libastro is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libastro. If not, see <http://www.gnu.org/licenses/>.
 */


/* TODO
christliche Feiertage
Advent
Sommer/Frühlings/Herbstanfang
Längster/Kürzester Tag
Vollmond
*/

#ifndef _TIME_H_
#define _TIME_H_

#include <stdbool.h>
#include <time.h>

/** Types */
struct date {
	int day;
	int month;
	int year;
};

struct time {
	int second;
	int minute;
	int hour;
};

/* helper to convert libastro types to struct tm */
struct tma {
	struct time time;
	struct date date;
	int wday;		/* day of the week */
	int yday;		/* day in the year */
	int isdst;		/* daylight saving time */
};

union datetime {
	struct tma libastro;
	struct tm posix;
};

enum weekday {
	MONDAY, TUESDAY, WEDNESDAY,
	THURSDAY, FRIDAY, SATURDAY, SUNDAY
};

enum holiday {
	EASTER_SUNDAY,
	CHRISTMAS,
	ADVENT_1, ADVENT_2, ADVENT_3, ADVENT_4,
	SILVESTER, NEWYEAR
};

/** Prototypes */

/** Helper functions */
double frac(double x);

/**
 * Compare to dates like strcmp()
 */
int datecmp(struct date a, struct date b);

struct date dateadd(struct date a, struct date b);


struct date holiday(struct date d, enum holiday type);

/**
 * Calculate the date of eastern
 *
 * @see http://de.wikipedia.org/wiki/Gau%C3%9Fsche_Osterformel
 */
struct date eastern(int year);

/**
 * Checks if year is a leapyear
 */
bool leapyear(int year);

/**
 * Calculate the daynumber of a gregorian date
 *  e.g the number of days after 1.1. of the current year
 */
int daynumber(struct date d);

/**
 * Calculate the weeknumber of a gregorian date
 *
 * according to DIN1355
 */
int weeknumber(struct date d);

/**
 * Return english weekday name
 */
int weekday(struct date d);

/**
 * Return sign
 */
const char * sign(struct date d, int mode);

/**
 * Convert a gregorian date to a julian date
 */
int gregorian2julian(struct date d);
struct date julian2gregorian(int jd);

/**
 * Convert HH:MM:SS to decimal day
 */
double hms2decimal(struct time t);
struct time decimal2hms(double decimal);

/**
 * Universial to Local Time
 */
//utc2local(struct date, int zone);

/**
 * Universal to Greenwich Sidereal Time
 */
//utc2gmst();

/**
 * Greenwich Sidereal to Universal Time
 */
//gmst2utc();

/**
 * Local Sidereal to Univeral Time
 */
//lmst2utc();

/**
 * Universal to Terrestrial (dynamic) Time
 */
//utc2tdt(utc);

/**
 * Ephimeris Time
 */
//struct time utc2et(struct time d);

#endif /* _TIME_H_ */
