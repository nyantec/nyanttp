#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <nyanttp/urldecode.h>

static char const enc[] = "%21%2A%27%28%29%3B%3A%40%26%3D%2B%24%2C%2F%3F%23%5B%5D";
static char const dec[] = "!*'();:@&=+$,/?#[]";

int main(int argc, char *argv[]) {
	char buf[sizeof dec - 1];

	ssize_t len = ny_urldecode(buf, enc, sizeof buf, sizeof enc - 1);
	assert(len == sizeof dec - 1);
	assert(!memcmp(buf, dec, len));

	return EXIT_SUCCESS;
}
