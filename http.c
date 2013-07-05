#include <assert.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>

#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include "nyanttp.h"
#include "http.h"

static struct addrinfo const hints = {
	.ai_flags =
		AI_PASSIVE |
		AI_V4MAPPED, /*< Map IPv4 addresses to IPv6 */
	.ai_family = AF_INET6,
	.ai_socktype = SOCK_STREAM,
	.ai_protocol = IPPROTO_TCP
};

int nyanttp_http_init(struct nyanttp_http *restrict http, char const *restrict node, char const *restrict service) {
	assert(http);

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

	ev_io_init(&http->io, nil, fd, EV_READ);

exit:
	if (likely(res))
		freeaddrinfo(res);

	return ret;
}
