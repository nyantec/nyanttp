#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <defy/nil>

#include "../ny.h"
#include "../alloc.h"

int main(int argc, char *argv[]) {
	uint_least32_t const num = 262144;
	uint_least16_t const len = 16;

	void **ptr = calloc(num, sizeof (void *));
	assert(ptr != nil);

	struct ny ny;
	ny_init(&ny);

	struct ny_alloc alloc;
	ny_alloc_init(&alloc, &ny, num, len);

	for (size_t pass = 0; pass < 64; ++pass) {
		/* Random acquire */
		for (size_t iter = 0; iter < num; ++iter) {
			if (!ptr[iter] && random() % 4 == 0) {
				ptr[iter] = ny_alloc_acquire(&alloc);
				assert(ptr[iter] != nil);
				memset(ptr[iter], 0x02, len);
			}
		}

		/* Random release */
		for (size_t iter = 0; iter < num; ++iter) {
			if (ptr[iter] && random() % 4 == 0) {
				ny_alloc_release(&alloc, ptr[iter]);
				ptr[iter] = nil;
			}
		}
	}

	ny_alloc_destroy(&alloc);

	return EXIT_SUCCESS;
}
