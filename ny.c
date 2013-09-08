/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <pthread.h>

#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include <ev.h>

#include <ny/ny.h>

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void atfork() {
	ev_loop_fork(EV_DEFAULT);
}

static void setup_once() {
	int _;

	/* Register fork handler */
	_ = pthread_atfork(nil, nil, atfork);
	assert(!_);
}

unsigned int ny_version_major() {
	return NY_VERSION_MAJOR;
}

unsigned int ny_version_minor() {
	return NY_VERSION_MINOR;
}

unsigned int ny_version_micro() {
	return NY_VERSION_MICRO;
}

/**
 * \brief Real context initialisation
 *
 * \param[out] ny Pointer to context structure
 *
 * \return Zero on success or non-zero on error
 */
int ny_init_(struct ny *restrict ny) {
	assert(ny);

	int _;
	int ret = 0;

	/* Check libev version */
	if (unlikely(ev_version_major() != EV_VERSION_MAJOR
		|| ev_version_minor() < EV_VERSION_MINOR)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVVER);
		ret = -1;
		goto exit;
	}

	/* Determine system page size */
	int page_size = sysconf(_SC_PAGE_SIZE);
	if (unlikely(page_size < 0)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		ret = -1;
		goto exit;
	}

	ny->page_size = page_size;

	/* Initialise default loop */
	ny->loop = ev_default_loop(EVFLAG_AUTO);
	if (unlikely(!ny->loop)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVINIT);
		ret = -1;
		goto exit;
	}

	/* One-time initialisation */
	_ = pthread_once(&once, setup_once);
	assert(!_);

exit:
	return ret;
}

void ny_destroy(struct ny *restrict ny) {
	assert(ny);

	assert(ny->loop);
	ny->loop = nil;
}
