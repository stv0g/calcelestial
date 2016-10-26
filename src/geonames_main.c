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
	struct ln_lnlat_posn res;
	char *result_name = malloc(32);
	char *name = "Aachen";

	if (result_name == NULL) {
		return EXIT_FAILURE;
	}

	if (argc == 2) {
		name = argv[1];
	}

	int ret = geonames_lookup(name, &res, result_name, 32);
	if (ret == EXIT_SUCCESS) {
		printf("%s is at (%.4f, %.4f)\r\n", result_name, res.lat, res.lng);
	}

	free(result_name);

	return ret;
}
