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

int nyanttp_init(struct nyanttp *restrict ctx) {
	assert(ctx);

	int _;
	int ret = 0;

	/* Initialise default loop */
	ctx->loop = ev_default_loop(EVFLAG_AUTO);
	if (unlikely(!ctx->loop)) {
		ret = -1;
		goto exit;
	}

	/* One-time initialisation */
	_ = pthread_once(&once, setup_once);
	assert(!_);

exit:
	return ret;
}
