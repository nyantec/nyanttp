#include "config.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <sys/types.h>

#include <nyanttp/expect.h>

ssize_t ny_urldecode(char *restrict out, char const *restrict in,
	size_t outlen, size_t inlen) {
	assert(out);
	assert(in);

	size_t itr;
	ssize_t otr;
	for (itr = otr = 0; itr < inlen && otr < outlen; ++itr, ++otr) {
		if (in[itr] == '%') {
			out[otr] = 0;

			for (uint_least8_t jtr = 0; jtr < 2; ++jtr) {
				switch (in[++itr]) {
				case '0' ... '9':
					out[otr] |= (in[itr] - '0' + 0x00) << (1 - jtr) * 4;
					break;

				case 'A' ... 'Z':
					out[otr] |= (in[itr] - 'A' + 0x0a) << (1 - jtr) * 4;
					break;

				case 'a' ... 'z':
					out[otr] |= (in[itr] - 'a' + 0x0a) << (1 - jtr) * 4;
					break;

				/* Invalid character */
				default:
					otr = -itr - 1;
					goto exit;
				}
			}
		}
		else
			out[otr] = in[itr];
	}

	if (unlikely(itr < inlen)) {
		otr = -(ssize_t) itr - 1;
		goto exit;
	}

exit:
	return otr;
}
