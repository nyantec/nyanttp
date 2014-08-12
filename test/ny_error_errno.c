#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <nyanttp/ny.h>
#include <nyanttp/error.h>

int main(int argc, char *argv[]) {
	struct ny_error error;
	ny_error_set(&error, NY_ERROR_DOMAIN_ERRNO, ENOTSUP);
	char const *str = ny_error(&error);
	assert(str != NULL);
	assert(strlen(str) > 0);

	return EXIT_SUCCESS;
}
