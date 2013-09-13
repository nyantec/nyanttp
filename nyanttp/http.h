#pragma once
#ifndef __ny_http__
#define __ny_http__

#if defined __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <ev.h>

#include <nyanttp/ny.h>
#include <nyanttp/error.h>
#include <nyanttp/alloc.h>
#include <nyanttp/tcp.h>

struct ny_http;
struct ny_http_con;
struct ny_http_req;

/**
 * \brief HTTP listener context
 */
struct ny_http {
	void *data; /**< User data */

	void (*req_readable)(struct ny_http_req *restrict);
	void (*req_writable)(struct ny_http_req *restrict);

	ssize_t (*recv)(void *restrict, void *restrict, size_t);
	ssize_t (*send)(void *restrict, void const *restrict, size_t);
};

/**
 * \brief HTTP connection context
 */
struct ny_http_con {
	void *data; /**< User data */
	void *ctx; /**< I/O context */

	struct ny_http *http;

	uint8_t *buffer; /**< Request buffer */
	size_t offset; /**< Buffer offset */
	size_t length; /**< Buffer capacity */
};

/**
 * \brief HTTP request context
 */
struct ny_http_req {
	void *data; /**< User data */
	struct ny_http_con *con; /**< HTTP connection */
};

extern int ny_http_init(struct ny_http *restrict http,
	struct ny *restrict ny);

extern int ny_http_con_init(struct ny_http_con *restrict con,
	struct ny_http *restrict http);

extern void ny_http_con_readable(struct ny_http_con *restrict con);

extern void ny_http_con_writable(struct ny_http_con *restrict con);

extern ssize_t ny_http_req_recv(struct ny_http_req *restrict req,
	void *restrict buffer, size_t length);

extern ssize_t ny_http_req_send(struct ny_http_req *restrict req,
	void const *restrict buffer, size_t length);

#if defined __cplusplus
}
#endif

#endif
