#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <defy/nil>

#include "../ny.h"
#include "../alloc.h"

int main(int argc, char *argv[]) {
	uint_least32_t const num = 262144;
	uint_least16_t const len = 64;

	uint8_t **ptr = calloc(num, sizeof (uint8_t *));
	assert(ptr != nil);

	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	ny_alloc_init(&alloc, &ny, num, len);

	/* Acquire objects */
	for (size_t iter = 0; iter < num; ++iter) {
		ptr[iter] = ny_alloc_acquire(&alloc);
		assert(ptr[iter] != nil);

		memset(ptr[iter], 0x02, len);
	}

	/* Release objects */
	for (size_t iter = 0; iter < num; ++iter)
		ny_alloc_release(&alloc, ptr[iter]);

	ny_alloc_destroy(&alloc);

	return EXIT_SUCCESS;
}
