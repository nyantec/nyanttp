/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <assert.h>
#include <string.h>

#include <netdb.h>

#include <gnutls/gnutls.h>

#include <nyanttp/local.h>
#include <nyanttp/error.h>
#include <nyanttp/expect.h>

/**
 * \brief Error message buffer
 */
static ny_local char error_buf[256];

/**
 * \brief Error message map
 */
static char const *const error_map[] = {
	[NY_ERROR_VERSION] = "ny version mismatch",
	[NY_ERROR_EVVER] = "libev version mismatch",
	[NY_ERROR_EVINIT] = "Failed to initialise libev event loop",
	[NY_ERROR_EVWATCH] = "libev watcher stopped",
	[NY_ERROR_EOF] = "End of file"
};

char const *ny_error_r(struct ny_error const *restrict error, char *restrict buffer, size_t length) {
	assert(error);
	assert(buffer);

	char const *string = "Unknown error";

	switch (error->domain) {
	case NY_ERROR_DOMAIN_NY:
		/* Check if error code inside bounds */
		if (unlikely(error->code >= sizeof error_map / sizeof *error_map))
			break;

		/* Check if error code is known */
		if (unlikely(!error_map[error->code]))
			break;

		string = error_map[error->code];
		break;

	case NY_ERROR_DOMAIN_ERRNO:
#if STRERROR_R_CHAR_P
		string = strerror_r(error->code, buffer, length);
#else
		if (unlikely(strerror_r(error->code, buffer, length)))
			break;

		string = buffer;
#endif
		break;

	case NY_ERROR_DOMAIN_GAI:
		string = gai_strerror(error->code);
		break;

	case NY_ERROR_DOMAIN_GTLS:
		string = gnutls_strerror(error->code);
		break;
	}

	return string;
}

char const *ny_error(struct ny_error const *restrict error) {
	return ny_error_r(error, error_buf, sizeof error_buf);
}
