#include <assert.h>
#include <stdlib.h>

#include <nyanttp/ny.h>

int main(int argc, char *argv[]) {
	struct ny ny;
	int _ = ny_init(&ny);
	assert(_ == 0);
	assert(ny.loop != NULL);
	assert(ny.page_size >= 0);

	return EXIT_SUCCESS;
}
