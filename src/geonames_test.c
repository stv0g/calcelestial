#include <stdio.h>

#include "geonames.h"

int main(int argc, char *argv[]) {
	struct coords res;
	char *name = "Aachen";

	if (argc == 2) {
		name = argv[1];
	}

	if (geonames_lookup(name, &res) == 0) {
		printf("%s is at (%.4f, %.4f)\r\n", name, res.lat, res.lon);
		return 0;
	}

	return 0;
}
