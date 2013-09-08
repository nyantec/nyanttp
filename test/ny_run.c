#include <assert.h>
#include <stdlib.h>

#include <defy/nil>

#include <ny/ny.h>

int main(int argc, char *argv[]) {
	struct ny ny;
	ny_init(&ny);

	int _ = ny_run(&ny, 4);
	assert(_ == 0);

	return EXIT_SUCCESS;
}
