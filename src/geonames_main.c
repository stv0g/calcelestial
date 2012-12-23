#include <stdio.h>
#include <stdlib.h>

#include "geonames.h"

int main(int argc, char *argv[]) {
	struct coords res;
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
		printf("%s is at (%.4f, %.4f)\r\n", result_name, res.lat, res.lon);
	}

	free(result_name);

	return ret;
}
