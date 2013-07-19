#include <pthread.h>

#include <assert.h>
#include <errno.h>

#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include <ev.h>

#include "nyanttp.h"

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

unsigned int nyanttp_version_major() {
	return NYANTTP_VERSION_MAJOR;
}

unsigned int nyanttp_version_minor() {
	return NYANTTP_VERSION_MINOR;
}

int nyanttp_init(struct nyanttp *restrict ctx) {
	assert(ctx);

	int _;
	int ret = 0;

	/* Check libev version */
	if (unlikely(ev_version_major() != EV_VERSION_MAJOR
		|| ev_version_minor() < EV_VERSION_MINOR)) {
		ctx->error.domain = NYANTTP_ERROR_DOMAIN_NYAN;
		ctx->error.code= NYANTTP_ERROR_EVVER;
		ret = -1;
		goto exit;
	}

	/* Initialise default loop */
	ctx->loop = ev_default_loop(EVFLAG_AUTO);
	if (unlikely(!ctx->loop)) {
		ctx->error.domain = NYANTTP_ERROR_DOMAIN_NYAN;
		ctx->error.code= NYANTTP_ERROR_EVINIT;
		ret = -1;
		goto exit;
	}

	/* One-time initialisation */
	_ = pthread_once(&once, setup_once);
	assert(!_);

exit:
	return ret;
}

void nyanttp_destroy(struct nyanttp *restrict ctx) {
	assert(ctx);

	assert(ctx->loop);
	ctx->loop = nil;
}
