#include <assert.h>
#include <stdlib.h>

#include <defy/nil>

#include "../ny.h"

int main(int argc, char *argv[]) {
	struct ny ny;
	ny_init(&ny);
	ny_destroy(&ny);
	assert(ny.loop == nil);

	return EXIT_SUCCESS;
}
