#include "config.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <sys/types.h>

#include <nyanttp/expect.h>

static char const enc[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

ssize_t ny_urlencode(char *restrict out, char const *restrict in,
	size_t outlen, size_t inlen) {
	assert(out);
	assert(in);

	size_t itr;
	ssize_t otr;
	for (itr = otr = 0; itr < inlen && otr < outlen; ++itr, ++otr) {
		switch (in[itr]) {
		case '-':
		case '.':
		case '0' ... '9':
		case '_':
		case 'A' ... 'Z':
		case 'a' ... 'z':
		case '~':
			out[otr] = in[itr];
			break;

		default:
			if (unlikely(outlen - otr < 3)) {
				otr = -(ssize_t) itr - 1;
				goto exit;
			}

			out[otr] = '%';
			out[++otr] = enc[(in[itr] & 0xf0u) >> 4];
			out[++otr] = enc[(in[itr] & 0x0fu) >> 0];
			break;
		}
	}

	if (unlikely(itr < inlen)) {
		otr = -(ssize_t) itr - 1;
		goto exit;
	}

exit:
	return otr;
}
