#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <defy/nil>

#include "../ny.h"
#include "../error.h"

int main(int argc, char *argv[]) {
	struct ny_error error;
	ny_error_set(&error, -1, -1);
	char const *str = ny_error(&error);
	assert(str != nil);
	assert(!strcmp(str, "Unknown error"));

	return EXIT_SUCCESS;
}
