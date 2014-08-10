#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <nyanttp/urldecode.h>

static char const enc[] = "RFC 398%6";

int main(int argc, char *argv[]) {
	char buf[sizeof enc];

	ssize_t len = ny_urldecode(buf, enc, sizeof buf, sizeof enc - 1);
	assert(len == -10);

	return EXIT_SUCCESS;
}
