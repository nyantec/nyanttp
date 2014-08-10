#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <nyanttp/urlencode.h>

static char const dec[] = "!*'();:@&=+$,/?#[]";

int main(int argc, char *argv[]) {
	char buf[sizeof dec];

	ssize_t len = ny_urlencode(buf, dec, sizeof buf, sizeof dec - 1);
	assert(len == -7);

	return EXIT_SUCCESS;
}
