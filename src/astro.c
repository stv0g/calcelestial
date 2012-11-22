#include <stdio.h>
#include <time.h>

#include "calendar.h"

extern const char* weekdays[];
extern const char* months[];

int main(int argc, char *argv[]) {
	time_t u = time(NULL);
	struct tm *n = localtime(&u); /* current time and date */

	struct date d;
	struct time t;

	if (argc == 2) {
		sscanf(argv[1], "%d.%d.%d", &d.day, &d.month, &d.year);
	}
	else {
		d.day = n->tm_mday;
		d.month = n->tm_mon+1;
		d.year = n->tm_year+1900;
	}

	t.hour = n->tm_hour;
	t.minute = n->tm_min;
	t.second = n->tm_sec;


	printf("Berechne für %d.%d.%d %d:%0d\n", d.day, d.month, d.year, t.hour, t.minute);

	struct date e = eastern(d.year);
	enum weekday wday = weekday(d);
	bool leap = leapyear(d.year);
	double dec = hms2decimal(t);
	int day = daynumber(d);
	int week = weeknumber(d); // TODO fix

	printf("In year %d, eastern is on %s the %d. %s\n", e.year, weekdays[weekday(e)], e.day, months[e.month-1]);
	printf("The year %d %s a leapyear\n", d.year, (leap) ? "is" : "is not");
	printf("Daynumber: %d\n", day);
	printf("Today is a %s\n", weekdays[wday]);
	printf("%.1f %% of the day are over :-%c\n", dec*1e2, (dec > 0.5) ? '(' : ')');
	printf("Weeknumber: %d\n", week);


	return 0;
}
