#include <assert.h>
#include <string.h>

#include <netdb.h>

#include <defy/expect>
#include <defy/thread_local>

#include "error.h"

/**
 * \brief Error message buffer
 */
static thread_local char error_buf[256];

/**
 * \brief Error message map
 */
static char const *const error_map[] = {
	[NYANTTP_ERROR_EVINIT] = "Failed to initialise libev event loop"
};

char const *nyanttp_error(struct nyanttp_error const *restrict error) {
	char const *string = "Unknown error";

	switch (error->domain) {
	case NYANTTP_ERROR_DOMAIN_NYAN:
		/* Check if error code inside bounds */
		if (unlikely(error->code.nyan >= sizeof error_map / sizeof *error_map))
			break;

		/* Check if error code is known */
		if (unlikely(!error_map[error->code.nyan]))
			break;

		string = error_map[error->code.nyan];
		break;

	case NYANTTP_ERROR_DOMAIN_ERRNO:
		if (unlikely(strerror_r(error->code.errno, error_buf, sizeof error_buf)))
			break;

		string = error_buf;
		break;

	case NYANTTP_ERROR_DOMAIN_GAI:
		string = gai_strerror(error->code.gai);
		break;
	}

	return string;
}
