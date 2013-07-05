#include <assert.h>
#include <errno.h>
#include <fcntl.h>
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

static void listen_event(EV_P_ ev_io io, int revents) {
	if (likely(revents | EV_READ)) {
		/* Accept up to 256 new connections */
		for (unsigned iter = 0; iter <= 255; ++iter) {
			int fd;
			do {
				fd = accept(io.fd, nil, nil);
			} while (fd < 0 && errno == EINTR);

			/* FIXME: Proper error handling */
			if (unlikely(fd < 0))
				break;

			/* TODO: Handle connection */
		}
	}
}

int nyanttp_tcp_init(struct nyanttp_tcp *restrict tcp, char const *restrict node, char const *restrict service) {
	assert(tcp);

	int _;
	int ret = 0;

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

		close(fd);
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
	ev_io_init(&tcp->io, nil, fd, EV_READ);

exit:
	if (likely(res))
		freeaddrinfo(res);

	return ret;
}

void nyanttp_tcp_destroy(struct nyanttp_tcp *restrict tcp) {
	assert(tcp);

	int _;

	/* Close socket */
	do {
		_ = close(tcp->io.fd);
	} while (_ && errno == EINTR);

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
