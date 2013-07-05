#include <assert.h>

#include <defy/nil>

#include "nyanttp.h"

void nyanttp_destroy(struct nyanttp *restrict ctx) {
	assert(ctx);

	assert(ctx->loop);
	ctx->loop = nil;
}
