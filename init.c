#include <assert.h>
#include <errno.h>

#include <defy/expect>
#include <defy/restrict>

#include <ev.h>

#include "nyanttp.h"

int nyanttp_init(struct nyanttp *restrict ctx) {
	assert(ctx);

	int ret = 0;

	/* Initialise default loop */
	ctx->loop = ev_default_loop(EVFLAG_AUTO);
	if (unlikely(!ctx->loop)) {
		ret = -1;
		goto exit;
	}

exit:
	return ret;
}
