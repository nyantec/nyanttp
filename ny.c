/**
 * \file
 *
 * \internal
 */

#include <pthread.h>

#include <assert.h>
#include <errno.h>

#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include <ev.h>

#include "ny.h"

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

unsigned int ny_version_patch() {
	return NY_VERSION_PATCH;
}

/**
 * \brief Real context initialisation
 *
 * \param[out] ctx Pointer to context structure
 *
 * \return Zero on success or non-zero on error
 */
int ny_init_(struct ny *restrict ctx) {
	assert(ctx);

	int _;
	int ret = 0;

	/* Check libev version */
	if (unlikely(ev_version_major() != EV_VERSION_MAJOR
		|| ev_version_minor() < EV_VERSION_MINOR)) {
		ny_error_set(&ctx->error, NY_ERROR_DOMAIN_NYAN, NY_ERROR_EVVER);
		ret = -1;
		goto exit;
	}

	/* Initialise default loop */
	ctx->loop = ev_default_loop(EVFLAG_AUTO);
	if (unlikely(!ctx->loop)) {
		ny_error_set(&ctx->error, NY_ERROR_DOMAIN_NYAN, NY_ERROR_EVINIT);
		ret = -1;
		goto exit;
	}

	/* One-time initialisation */
	_ = pthread_once(&once, setup_once);
	assert(!_);

exit:
	return ret;
}

void ny_destroy(struct ny *restrict ctx) {
	assert(ctx);

	assert(ctx->loop);
	ctx->loop = nil;
}