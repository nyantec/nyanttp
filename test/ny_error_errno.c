#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <defy/nil>

#include <ny/ny.h>
#include <ny/error.h>

int main(int argc, char *argv[]) {
	struct ny_error error;
	ny_error_set(&error, NY_ERROR_DOMAIN_ERRNO, ENOTSUP);
	char const *str = ny_error(&error);
	assert(str != nil);
	assert(strlen(str) > 0);

	return EXIT_SUCCESS;
}
