#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>

#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include "nyanttp.h"
#include "tcp.h"

static struct addrinfo const hints = {
	.ai_flags =
		AI_PASSIVE |
		AI_V4MAPPED, /*< Map IPv4 addresses to IPv6 */
	.ai_family = AF_INET6,
	.ai_socktype = SOCK_STREAM,
	.ai_protocol = IPPROTO_TCP
};

static int safe_close(int fd) {
	int ret;

	do {
		ret = close(fd);
	} while (ret && errno == EINTR);

	return ret;
}

static int safe_accept(int fd, struct sockaddr *restrict address, socklen_t *restrict addrlen) {
	int ret;

	do {
		ret = accept(fd, address, addrlen);
	} while (ret < 0 && errno == EINTR);

	return ret;
}

static void conn_event(EV_P_ ev_io io, int revents) {
	struct nyanttp_tcp_conn *conn = (struct nyanttp_tcp_conn *) io.data;

	if (revents & EV_READ) {
		conn->tcp->event_readable(conn);
	}

	if (revents & EV_WRITE) {
		conn->tcp->event_writable(conn);
	}
}

static void listen_event(EV_P_ ev_io io, int revents) {
	struct nyanttp_tcp *tcp = (struct nyanttp_tcp *) io.data;

	if (likely(revents & EV_READ)) {
		/* Accept up to 256 new connections */
		for (unsigned iter = 0; iter <= 255; ++iter) {
			struct sockaddr_in6 address;
			socklen_t addrlen = sizeof address;

			/* Accept connection */
			int fd = safe_accept(io.fd, (struct sockaddr *) &address, &addrlen);

			/* Assume that all addresses are IPv6 */
			assert(address.sin6_family == AF_INET6);

			/* FIXME: Proper error handling */
			if (unlikely(fd < 0))
				break;

			/* Raise connect event */
			if (tcp->event_connect) {
				if (unlikely(!tcp->event_connect(tcp, &address))) {
					/* Reject connection */
					safe_close(fd);
					continue;
				}
			}

			/* Allocate memory for connection structure */
			struct nyanttp_tcp_conn *conn = malloc(sizeof (struct nyanttp_tcp_conn));
			if (unlikely(!conn)) {
				/* TODO: Handle error */
			}

			/* Initialise event watcher */
			ev_io_init(&conn->io, conn_event, fd, EV_READ);
			conn->io.data = conn;
			ev_io_start(EV_A_ &conn->io);
		}
	}
}

int nyanttp_tcp_init(struct nyanttp_tcp *restrict tcp, char const *restrict node, char const *restrict service) {
	assert(tcp);

	int _;
	int ret = 0;

	/* Initialise structure */
	tcp->data = nil;
	tcp->event_connect = nil;

	struct addrinfo *res;

	_ = getaddrinfo(node, service, &hints, &res);
	if (unlikely(_)) {
		ret = -1;
		goto exit;
	}

	int fd = -1;
	for (struct addrinfo *rp = res; rp; rp = rp->ai_next) {
		/* Create socket */
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (unlikely(fd < 0))
			continue;

		/* Bind to socket */
		if (likely(!bind(fd, rp->ai_addr, rp->ai_addrlen)))
			break;

		safe_close(fd);
	}

	if (fd < 0) {
		ret = -1;
		goto exit;
	}

	/* Close socket on exec */
	_ = fcntl(fd, F_GETFD);
	assert(_ >= 0);
	_ = fcntl(fd, F_SETFD, _ | FD_CLOEXEC);
	assert(_);

	/* Mark socket as non-blocking */
	_ = fcntl(fd, F_GETFL);
	assert(_ >= 0);
	_ = fcntl(fd, F_SETFL, _ | O_NONBLOCK);
	assert(_);

	/* Initialise I/O watcher */
	ev_io_init(&tcp->io, listen_event, fd, EV_READ);
	tcp->io.data = tcp;

exit:
	if (likely(res))
		freeaddrinfo(res);

	return ret;
}

void nyanttp_tcp_destroy(struct nyanttp_tcp *restrict tcp) {
	assert(tcp);

	int _;

	/* Close socket */
	_ = safe_close(tcp->io.fd);

	assert(_);
}

int nyanttp_tcp_listen(struct nyanttp_tcp *restrict tcp, struct nyanttp *restrict ctx) {
	assert(tcp);
	assert(ctx);

	int ret = 0;

	/* Listen on socket */
	if (listen(tcp->io.fd, SOMAXCONN)) {
		ret = errno;
		goto exit;
	}

	ev_io_start(ctx->loop, &tcp->io);

exit:
	return ret;
}
