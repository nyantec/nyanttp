/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include <nyanttp/ny.h>
#include <nyanttp/expect.h>
#include <nyanttp/tcp.h>
#include <nyanttp/io.h>

/**
 * \brief Address resolution hints
 */
static struct addrinfo const hints = {
	.ai_flags =
		AI_PASSIVE |
		AI_V4MAPPED, /*< Map IPv4 addresses to IPv6 */
	.ai_family = AF_INET6,
	.ai_socktype = SOCK_STREAM,
	.ai_protocol = IPPROTO_TCP
};

static int goat_new() {
	int goat = fcntl(0, F_DUPFD, 0);
	assert(goat >= 0);

	/* Close on exec */
	ny_io_fd_set(goat, FD_CLOEXEC);

	return goat;
}

static int accept_safe(struct ny_tcp *restrict tcp,
	struct sockaddr *restrict address, socklen_t *restrict addrlen) {
	int _;

	int csock = ny_io_accept(tcp->io.fd, address, addrlen);

	/* Reject connection if out of file descriptors */
	if (unlikely(csock < 0 && (errno == EMFILE || errno == ENFILE))) {
		int save = errno;
		_ = ny_io_close(tcp->goat);
		assert(!_);

		csock = ny_io_accept(tcp->io.fd, address, addrlen);
		if (likely(csock >= 0))
			ny_io_close(csock);

		tcp->goat = goat_new();
		errno = save;
	}

	return csock;
}

static void con_event(EV_P_ struct ev_io *io, int revents) {
	struct ny_tcp_con *con = (struct ny_tcp_con *) io->data;

	if (unlikely(revents & EV_ERROR)) {
		struct ny_error error;
		ny_error_set(&error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVWATCH);
		con->tcp->con_error(con, &error);
	}
	else {
		if (revents & EV_READ) {
			if (con->tcp->con_readable)
				con->tcp->con_readable(con);
		}

		if (revents & EV_WRITE) {
			if (con->tcp->con_writable)
				con->tcp->con_writable(con);
		}
	}
}

static void timeout_event(EV_P_ struct ev_timer *timer, int revents) {
	struct ny_tcp_con *con = (struct ny_tcp_con *) timer->data;

	if (unlikely(revents & EV_ERROR)) {
		struct ny_error error;
		ny_error_set(&error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVWATCH);
		con->tcp->con_error(con, &error);
	}
	else if (likely(revents & EV_TIMEOUT)) {
		/* Raise timeout event */
		if (con->tcp->con_timeout) {
			if (unlikely(con->tcp->con_timeout(con))) {
				ny_tcp_con_touch(con);
				return;
			}
		}

		ny_tcp_con_destroy(con);
	}
}

static void listen_event(EV_P_ struct ev_io *io, int revents) {
	struct ny_tcp *tcp = (struct ny_tcp *) io->data;

	if (unlikely(revents & EV_ERROR)) {
		struct ny_error error;
		ny_error_set(&error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVWATCH);
		tcp->tcp_error(tcp, &error);
	}

	else if (likely(revents & EV_READ)) {
		for (unsigned iter = 0; iter < NY_TCP_ACCEPT_MAX; ++iter) {
			struct sockaddr_in6 address;
			socklen_t addrlen = sizeof address;

			/* Accept connection */
			int fd = accept_safe(tcp, (struct sockaddr *) &address, &addrlen);

			/* Handle errors */
			if (unlikely(fd < 0)) {
				if (unlikely(errno != EAGAIN || errno != EWOULDBLOCK)) {
					if (tcp->tcp_error) {
						struct ny_error error;
						ny_error_set(&error, NY_ERROR_DOMAIN_ERRNO, errno);
						tcp->tcp_error(tcp, &error);
					}
				}

				break;
			}

			/* Assume that all addresses are IPv6 */
			assert(address.sin6_family == AF_INET6);

			/* Raise conect event */
			if (tcp->tcp_connect) {
				if (unlikely(!tcp->tcp_connect(tcp, &address))) {
					/* Reject connection */
					ny_io_close(fd);
					continue;
				}
			}

			/* Allocate memory for connection structure */
			struct ny_tcp_con *con = ny_alloc_acquire(&tcp->alloc_con);
			if (unlikely(!con)) {
				if (tcp->tcp_error)
					tcp->tcp_error(tcp, &tcp->ny->error);

				/* Close connection */
				ny_io_close(fd);
				break;
			}

			/* Initialise user-data pointer */
			con->data = NULL;

			/* Set TCP context pointer */
			con->tcp = tcp;

			/* Initialise timeout watcher */
			ev_timer_init(&con->timer, timeout_event,
				NY_TCP_TIMEOUT, NY_TCP_TIMEOUT);
			ev_set_priority(&con->timer, NY_TCP_TIMER_PRIO);
			con->timer.data = con;
			ev_timer_start(EV_A_ &con->timer);

			/* Initialise I/O watcher */
			ev_io_init(&con->io, con_event, fd, EV_READ);
			ev_set_priority(&con->io, NY_TCP_ACCEPT_PRIO);
			con->io.data = con;
			ev_io_start(EV_A_ &con->io);

			/* Emit connect event */
			if (tcp->con_connect)
				tcp->con_connect(con);
		}
	}
}

int ny_tcp_init(struct ny_tcp *restrict tcp, struct ny *restrict ny,
	char const *restrict node, char const *restrict service,
	uint_least32_t maxcon) {
	assert(tcp);
	assert(ny);

	int _;
	int ret = -1;

	/* Initialise structure */
	tcp->data = NULL;
	tcp->ny = ny;
	tcp->tcp_error = NULL;
	tcp->tcp_connect = NULL;
	tcp->con_error = NULL;
	tcp->con_destroy = NULL;
	tcp->con_connect = NULL;
	tcp->con_readable = NULL;
	tcp->con_writable = NULL;
	tcp->con_timeout = NULL;

	struct addrinfo *res;

	_ = getaddrinfo(node, service, &hints, &res);
	if (unlikely(_)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_GAI, _);
		goto exit;
	}

	int fd = -1;
	for (struct addrinfo *rp = res; rp; rp = rp->ai_next) {
		/* Create socket */
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (unlikely(fd < 0)) {
			ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
			continue;
		}

		/* Allow address reuse */
		int value = 1;
		if (unlikely(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value,
			sizeof (value)))) {
			ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
			ny_io_close(fd);
			continue;
		}

		/* Bind to socket */
		if (likely(!bind(fd, rp->ai_addr, rp->ai_addrlen)))
			break;

		ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		ny_io_close(fd);
	}

	if (fd < 0)
		goto exit;

	/* Mark socket as non-blocking */
	ny_io_fl_set(fd, O_NONBLOCK);

	/* Close socket on exec */
	ny_io_fd_set(fd, FD_CLOEXEC);

	/* Initialise I/O watcher */
	ev_io_init(&tcp->io, listen_event, fd, EV_READ);
	ev_set_priority(&tcp->io, NY_TCP_IO_PRIO);
	tcp->io.data = tcp;

	/* Initialise allocator */
	_ = ny_alloc_init(&tcp->alloc_con, ny, maxcon, sizeof (struct ny_tcp_con));
	if (unlikely(_))
		goto exit;

	/* Duplicate standard input as a reserve file descriptor */
	tcp->goat = goat_new();

	ret = 0;

exit:
	if (likely(res))
		freeaddrinfo(res);

	return ret;
}

void ny_tcp_destroy(struct ny_tcp *restrict tcp) {
	assert(tcp);

	int _;

	/* Close socket */
	_ = ny_io_close(tcp->io.fd);
	assert(_);

	/* Destroy allocator */
	ny_alloc_destroy(&tcp->alloc_con);

	/* Close reserve file descriptor */
	_ = ny_io_close(tcp->goat);
	assert(_);
}

int ny_tcp_listen(struct ny_tcp *restrict tcp) {
	assert(tcp);

	int ret = -1;

	/* Listen on socket */
	if (listen(tcp->io.fd, SOMAXCONN)) {
		ny_error_set(&tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		goto exit;
	}

	ev_io_start(tcp->ny->loop, &tcp->io);

	ret = 0;

exit:
	return ret;
}

void ny_tcp_con_destroy(struct ny_tcp_con *restrict con) {
	assert(con);

	/* Call destroy event handler */
	if (con->tcp->con_destroy)
		con->tcp->con_destroy(con);

	/* Stop watchers */
	ev_io_stop(con->tcp->ny->loop, &con->io);
	ev_timer_stop(con->tcp->ny->loop, &con->timer);

	/* Close socket */
	ny_io_close(con->io.fd);

	/* Free structure */
	ny_alloc_release(&con->tcp->alloc_con, con);
}

void ny_tcp_con_touch(struct ny_tcp_con *restrict con) {
	assert(con);

	/* Reset timeout timer */
	ev_timer_again(con->tcp->ny->loop, &con->timer);
}

void ny_tcp_con_events(struct ny_tcp_con *restrict con, int events) {
	assert(con);
	assert(events);
	assert((events & ~(NY_TCP_READABLE | NY_TCP_WRITABLE)) == 0);

	ev_io_stop(con->tcp->ny->loop, &con->io);
	ev_io_set(&con->io, &con->io.fd, events);
	ev_io_start(con->tcp->ny->loop, &con->io);
}

ssize_t ny_tcp_con_recv(struct ny_tcp_con *restrict con,
	void *restrict buffer, size_t length) {
	assert(con);
	assert(buffer);

	ssize_t rlen = ny_io_read(con->io.fd, buffer, length);
	if (unlikely(rlen == 0)) {
		rlen = -1;
		ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_NY, NY_ERROR_EOF);
	}
	else if (unlikely(rlen < 0)) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			rlen = 0;
		else
			ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
	}

	/* Reset timeout */
	else if (likely(rlen > 0))
		ny_tcp_con_touch(con);

	return rlen;
}

ssize_t ny_tcp_con_send(struct ny_tcp_con *restrict con,
	void const *restrict buffer, size_t length) {
	assert(con);
	assert(buffer);

	ssize_t wlen = ny_io_write(con->io.fd, buffer, length);
	if (unlikely(wlen < 0)) {
		if (errno == EINTR || errno == EWOULDBLOCK)
			wlen = 0;
		else
			ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
	}

	/* Reset timeout */
	else if (likely(wlen > 0))
		ny_tcp_con_touch(con);

	return wlen;
}

ssize_t ny_tcp_con_sendfile(struct ny_tcp_con *restrict con,
	int fd, size_t length, off_t offset) {
	assert(con);
	assert(fd >= 0);

	ssize_t wlen = ny_io_sendfile(con->io.fd, fd, length, offset);
	if (unlikely(wlen < 0)) {
		if (errno == EINTR || errno == EWOULDBLOCK)
			wlen = 0;
		else
			ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
	}

	/* Reset timeout */
	else if (likely(wlen > 0))
		ny_tcp_con_touch(con);

	return wlen;
}
