#include <assert.h>

#include <defy/nil>

#include "nyanttp.h"

int nyanttp_destroy(struct nyanttp *restrict ctx) {
	assert(ctx);

	int ret = 0;

	assert(ctx->loop);
	ctx->loop = nil;

	return ret;
}
