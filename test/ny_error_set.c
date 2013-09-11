#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <defy/nil>

#include <nyanttp/ny.h>
#include <nyanttp/error.h>

int main(int argc, char *argv[]) {
	struct ny_error error;
	ny_error_set(&error, NY_ERROR_DOMAIN_ERRNO, ENOTSUP);
	assert(error.file != nil);
	assert(error.func != nil);
	assert(!strcmp(error.file, __FILE__));
	assert(!strcmp(error.func, __func__));
	assert(error.line == __LINE__ - 5);
	assert(error.domain == NY_ERROR_DOMAIN_ERRNO);
	assert(error.code == ENOTSUP);

	return EXIT_SUCCESS;
}
