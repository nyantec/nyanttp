#include <assert.h>
#include <stdlib.h>

#include <nyanttp/ny.h>
#include <nyanttp/alloc.h>

int main(int argc, char *argv[]) {
	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	ny_alloc_init(&alloc, &ny, 1021, 23);

	ny_alloc_destroy(&alloc);
	assert(alloc.pool == NULL);
	assert(alloc.memsize == 0);

	return EXIT_SUCCESS;
}
