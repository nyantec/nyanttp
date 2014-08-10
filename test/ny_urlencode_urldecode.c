#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <nyanttp/urldecode.h>
#include <nyanttp/urlencode.h>

static char dec[1024];
static char enc[3 * sizeof dec];
static char buf[sizeof dec];

int main(int argc, char *argv[]) {
	for (size_t itr = 0; itr < sizeof dec; ++itr)
		dec[itr] = random() % 0xff;

	ssize_t enclen = ny_urlencode(enc, dec, sizeof enc, sizeof dec);
	assert(enclen >= sizeof dec);

	ssize_t declen = ny_urldecode(buf, enc, sizeof buf, enclen);
	assert(declen == sizeof dec);

	assert(!memcmp(dec, buf, sizeof dec));

	return EXIT_SUCCESS;
}
