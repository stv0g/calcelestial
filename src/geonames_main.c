/**
 * Geonames.org test
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

#include <stdio.h>
#include <stdlib.h>

#include <libnova/libnova.h>

#include "../config.h"
#include "geonames.h"

int main(int argc, char *argv[]) {
	int ret, gmt_offset;
	struct ln_lnlat_posn res;
	char name[128], tzid[32];

	if (argc != 2)
		fprintf(stderr, "Usage: geonames LOCATION\n");

	ret = geonames_lookup_latlng(argv[1], &res, name, sizeof(name));
	if (ret) {
		fprintf(stderr, "Error: Failed to lookup coordinates of %s\n", argv[1]);
		return 1;
	}
	
	ret = geonames_lookup_tz(res, &gmt_offset, tzid, sizeof(tzid));
	if (ret)
		fprintf(stderr, "Error: Failed to lookup timezone for %.4f %.4f\n", res.lat, res.lng);
	
	printf("%s is at %.4f, %.4f with timezone %s (GMT%+d)\r\n", name, res.lat, res.lng, tzid, gmt_offset);

	return ret;
}
