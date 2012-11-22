#include <math.h>

#include "calendar.h"

/** Constants */
const double DELTA_T = 66.7; /* seconds */

const char *weekdays[] = { "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday", "Sunday" };

const char *holidays[] = { };

const char *months[] = { "January", "February", "March",
	"April", "May", "June", "July", "August",
	"October", "November", "December" };

const char *signs[] = { "Aries", "Taurus", "Gemini", "Cancer",
	"Leo", "Virgo", "Libra", "Scorpio",
	"Ophiuchus", "Sagittarius", "Capricorn",
	"Aquarius", "Pisces" };

double frac(double x) {
	return x - floor(x);
}

struct date eastern(int year) {
	int k = year / 100;				/* Säkularzahl */
	int m = 15 + (3*k + 3) / 4 - (8*k + 13) / 25;	/* säkulare Mondschaltung */
	int s = 2 - (3*k + 3) / 4;			/* säkulare Sonnenschaltung */
	int a = year % 19;				/* den Mondparameter */
	int d = (19*a + m) % 30;			/* Keim für den ersten Vollmond im Frühling */
	int r = (d + a/11) / 29;			/* kalendarische Korrekturgröße */
	int og = 21 + d - r;				/* Ostergrenze */
	int sz = 7 - (year + year/4 + s) % 7;		/* erster Sonntag im März */
	int oe = 7 - (og - sz) % 7;			/* Osterentfernung in Tagen */
	int os = os = og + oe;				/* Datum des Ostersonntags als Märzdatum */

	struct date eastern = {
		.year = year,
		.month = (os > 31) ? 4 : 3,
		.day = (os > 31) ? os - 31 : os
	};

	return eastern;
}

struct date holiday(struct date date, enum holiday type) {
	switch (type) {
		case EASTER_SUNDAY:
			return eastern(date.year);
			break;

		case NEWYEAR: {
			struct date ny;
			ny.year = date.year;
			ny.month = 1;
			ny.day = 1;
			return ny;
		}

		case SILVESTER: {
			struct date sv;
			sv.year = date.year;
			sv.month = 12;
			sv.day = 31;
			return sv;
		}
	}
}

int weekday(struct date date) {
	const int century_code[] = { 6, 4, 2, 0 };
	const int month_code[] = { 6, 2, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

	int l = (leapyear(date.year) && date.month <= 2) ? 1 : 0;
	int x = date.year % 100;
	int y = date.year / 100;
	int a = century_code[(y - 16) % 4];
	int b = month_code[(date.month-1) % 12] - l;
	int c = x + x/4;
	int d = date.day;

	return (a + b + c + d) % 7;
}

bool leapyear(int year) {
	if ((year % 400) == 0)
		return true;
	else if ((year % 100) == 0)
		return false;
	else if ((year % 4) == 0)
		return true;
	else
		return false;
}

int daynumber(struct date date) {
	int days = date.month;

	if (date.month <= 2) {
		days--;
		days *= leapyear(date.year) ? 62 : 63;
		days /= 2;
	}
	else {
		days++;
		days *= 30.6;
		days -= leapyear(date.year)? 62 : 63;
	}

	return days + date.day;
}

int weeknumber(struct date date) {
	int dn = daynumber(date);

	struct date ny = { /* new year */
		.year = date.year,
		.month = 1, .day = 1
	};
	int wdny = weekday(ny);

	// Sonderfälle Freitag und Samstag
	if (wdny >= 5) {
		wdny -= 7;
	}

	// Sonderfälle "Jahresanfang mit KW-Nummer aus dem Vorjahr"
	if (dn + wdny <= 1) {
		struct date sv = { /* silvester last year */
			.year = date.year - 1,
			.month = 12, .day = 31
		};
		return weeknumber(sv);
	}

	int wn = (dn + wdny + 5) / 7;

	/* 53 Kalenderwochen hat grundsätzlich nur ein Jahr, 
	   welches mit einem Donnerstag anfängt !
	   In Schaltjahren ist es auch mit einem Mittwoch möglich, z.B. 1992
	   Andernfalls ist diese KW schon die KW1 des Folgejahres. */
	if (wn == 53) {
		bool ly = leapyear(date.year);

		if ( (wdny + 7 ) % 7 ==  4 || ((wdny + 7) % 7 == 3 && ly)) {
			return wn;
		}
		else {
			return 1; // Korrektur des Wertes
		}
	}
	else {
		return wn;
	}
}

double hms2decimal(struct time t) {
	return (((t.second / 60.0) + t.minute) / 60.0 + t.hour) / 24.0;
}

struct time decimal2hms(double decimal) {
	double hours = decimal * 24;
	double minutes = frac(hours) * 60;
	double seconds = frac(minutes) * 60;

	struct time time = { hours, minutes, seconds };
	return time;
}

int gregorian2julian(struct date d) {
	if (d.month <= 2) {
		d.year--;
		d.month += 12;
	}

	struct date x = { .day = 15, .month = 10, .year = 1582 };
	if (datecmp(d, x) > 0) {
		int a = d.year / 100;
		int b = 2 - a + 
	}

	return b + c + d + d.day + 1720994.5
}

struct date julian2gregorian(int jd) {

}

struct time utc2et(struct time d) {
	
}

int datecmp(struct date a, struct date b) {
	if (a.year == b.year) {
		if (a.month == b.month) {
			if (a.day == b.day) return 0;
			else return a.day - b.day;
		}
		else return a.month - b.month;
	}
	else return a.year - b.year;
}
