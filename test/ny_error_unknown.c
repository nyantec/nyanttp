#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <nyanttp/ny.h>
#include <nyanttp/error.h>

int main(int argc, char *argv[]) {
	struct ny_error error;
	ny_error_set(&error, -1, -1);
	char const *str = ny_error(&error);
	assert(str != NULL);
	assert(!strcmp(str, "Unknown error"));

	return EXIT_SUCCESS;
}
