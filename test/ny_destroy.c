#include <assert.h>
#include <stdlib.h>

#include <nyanttp/ny.h>

int main(int argc, char *argv[]) {
	struct ny ny;
	ny_init(&ny);
	ny_destroy(&ny);
	assert(ny.loop == NULL);

	return EXIT_SUCCESS;
}
