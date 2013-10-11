/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include <defy/expect>
#include <defy/nil>

#include <nyanttp/ny.h>
#include <nyanttp/http.h>

int ny_http_init(struct ny_http *restrict http,
	struct ny *restrict ny) {
	assert(http);
	assert(ny);

	int _;
	int status = -1;

	http->data = nil;

	status = 0;

exit:
	return status;
}

int ny_http_con_init(struct ny_http_con *restrict con,
	struct ny_http *restrict http) {
	assert(con);
	assert(http);

	con->data = nil;
	con->http = http;

	con->buffer = nil;
	con->offset = 0;
	con->length = 0;
}

void ny_http_con_readable(struct ny_http_con *restrict con) {
	assert(con);

	/* FIXME: Pass body through */

	/* Resize buffer if necessary */
	if (con->length - con->offset <= con->length / 4) {
		size_t length = con->length ? 2 * con->length : 512;
		/* FIXME: Error if request too large */
		uint8_t *buffer = realloc(con->buffer, length);
		if (unlikely(!buffer)) {
			/* TODO: Handle error */
		}

		con->buffer = buffer;
		con->length = length;
	}

	size_t rlen = con->http->recv(con->ctx, con->buffer + con->offset,
		con->length - con->offset);
	if (unlikely(rlen < 0)) {
		/* TODO: Handle error */
	}

	con->offset += rlen;

	/* TODO: Parse data */
}

void ny_http_con_writable(struct ny_http_con *restrict con) {
}

ssize_t ny_http_req_recv(struct ny_http_req *restrict req,
	void *restrict buffer, size_t length) {

	/* TODO: Get partial body from buffer and pass the rest through */
}

ssize_t ny_http_req_send(struct ny_http_req *restrict req,
	void const *restrict buffer, size_t length) {
}
