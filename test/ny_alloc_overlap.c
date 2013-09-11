#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <defy/nil>

#include <nyanttp/ny.h>
#include <nyanttp/alloc.h>

int main(int argc, char *argv[]) {
	uint_least32_t const num = 256;
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

		/* Check for overlaps */
		memset(ptr[iter], 0xff, len);
		for (size_t jter = 0; jter < iter; ++jter)
			for (size_t kter = 0; kter < len; ++kter)
				assert(ptr[jter][kter] == 0x00);
		memset(ptr[iter], 0x00, len);
	}

	ny_alloc_destroy(&alloc);

	return EXIT_SUCCESS;
}
