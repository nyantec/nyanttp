#include <pthread.h>

#include <assert.h>
#include <errno.h>

#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include <ev.h>

#include "nyanttp.h"

static void atfork() {
	ev_loop_fork(EV_DEFAULT);
}

int nyanttp_init(struct nyanttp *restrict ctx) {
	assert(ctx);

	int ret = 0;

	/* Initialise default loop */
	ctx->loop = ev_default_loop(EVFLAG_AUTO);
	if (unlikely(!ctx->loop)) {
		ret = -1;
		goto exit;
	}

	/* Register fork handler */
	int pth_ret = pthread_atfork(nil, nil, atfork);
	if (unlikely(pth_ret)) {
		ret = pth_ret;
		goto exit;
	}

exit:
	return ret;
}
