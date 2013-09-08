#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <defy/nil>

#include <ny/ny.h>
#include <ny/alloc.h>

int main(int argc, char *argv[]) {
	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	int _ = ny_alloc_init(&alloc, &ny, 1021, 23);
	assert(_ == 0);
	assert(alloc.raw != nil && alloc.raw != MAP_FAILED);
#if HAVE_MMAP
	assert(alloc.pool == alloc.raw + ny.page_size);
	assert(alloc.alloc >= 3 * ny.page_size);
	assert(alloc.alloc % ny.page_size == 0);
#else
	assert(alloc.pool == alloc.raw);
#endif
	assert(alloc.free != UINT32_MAX);
	assert(alloc.size >= 4);
	assert(alloc.size % sizeof (void *) == 0);

	return EXIT_SUCCESS;
}
