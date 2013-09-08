#include <assert.h>
#include <stdlib.h>

#include <ny/ny.h>

int main(int argc, char *argv[]) {
	assert(ny_version_major() == NY_VERSION_MAJOR);
	assert(ny_version_minor() == NY_VERSION_MINOR);
	assert(ny_version_micro() == NY_VERSION_MICRO);
	return EXIT_SUCCESS;
}
