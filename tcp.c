/**
 * \file
 *
 * \internal
 */

#include "config.h"

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

#include <ny/ny.h>
#include <ny/tcp.h>

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

/**
 * \brief Close file descriptor, retrying if interrupted
 *
 * \param[in] fd File descriptor to close
 *
 * \return Zero on success or non-zero on failure
 */
static int safe_close(int fd) {
	int ret;

	do {
		ret = close(fd);
	} while (ret && errno == EINTR);

	return ret;
}

/**
 * \brief Accept new connection, retrying if interrupted
 *
 * \param[in] fd File descriptor of the listening socket
 * \param[out] address Pointer to a \c sockaddr structure to hold the address of the connecting socket
 * \param[in,out] addrlen Capacity of the structure given by \p address on input, the actual length of the stored address on output
 *
 * \return Non-negative file descriptor of the accepted socket or a negative integer on failure
 */
static int safe_accept(int fd, struct sockaddr *restrict address, socklen_t *restrict addrlen) {
	int ret;

	do {
		ret = accept(fd, address, addrlen);
	} while (ret < 0 && errno == EINTR);

	return ret;
}

static void conn_event(EV_P_ ev_io io, int revents) {
	struct ny_tcp_conn *conn = (struct ny_tcp_conn *) io.data;

	if (unlikely(revents & EV_ERROR)) {
		struct ny_error error;
		ny_error_set(&error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVWATCH);
		conn->tcp->event_conn_error(conn, &error);
	}
	else {
		if (revents & EV_READ) {
			if (conn->tcp->event_conn_readable)
				conn->tcp->event_conn_readable(conn);
		}

		if (revents & EV_WRITE) {
			if (conn->tcp->event_conn_writable)
				conn->tcp->event_conn_writable(conn);
		}
	}
}

static void timeout_event(EV_P_ ev_timer timer, int revents) {
	struct ny_tcp_conn *conn = (struct ny_tcp_conn *) timer.data;

	if (unlikely(revents & EV_ERROR)) {
		struct ny_error error;
		ny_error_set(&error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVWATCH);
		conn->tcp->event_conn_error(conn, &error);
	}
	else if (likely(revents & EV_TIMEOUT)) {
		/* Raise timeout event */
		if (conn->tcp->event_conn_timeout)
			if (unlikely(conn->tcp->event_conn_timeout(conn)))
				return;

		ny_tcp_conn_destroy(conn);
	}
}

static void listen_event(EV_P_ ev_io io, int revents) {
	struct ny_tcp *tcp = (struct ny_tcp *) io.data;

	if (unlikely(revents & EV_ERROR)) {
		struct ny_error error;
		ny_error_set(&error, NY_ERROR_DOMAIN_NY, NY_ERROR_EVWATCH);
		tcp->event_tcp_error(tcp, &error);
	}
	else if (likely(revents & EV_READ)) {
		for (unsigned iter = 0; iter < NY_TCP_ACCEPT_MAX; ++iter) {
			struct sockaddr_in6 address;
			socklen_t addrlen = sizeof address;

			/* Accept connection */
			int fd = safe_accept(io.fd, (struct sockaddr *) &address, &addrlen);

			/* Assume that all addresses are IPv6 */
			assert(address.sin6_family == AF_INET6);

			/* Handle errors */
			if (unlikely(fd < 0)) {
				if (unlikely(errno != EAGAIN && errno != EWOULDBLOCK)) {
					if (tcp->event_tcp_error) {
						struct ny_error error;
						ny_error_set(&error, NY_ERROR_DOMAIN_ERRNO, errno);
						tcp->event_tcp_error(tcp, &error);
					}
				}

				break;
			}

			/* Raise connect event */
			if (tcp->event_tcp_connect) {
				if (unlikely(!tcp->event_tcp_connect(tcp, &address))) {
					/* Reject connection */
					safe_close(fd);
					continue;
				}
			}

			/* Allocate memory for connection structure */
			struct ny_tcp_conn *conn = malloc(sizeof (struct ny_tcp_conn));
			if (unlikely(!conn)) {
				if (tcp->event_tcp_error) {
					struct ny_error error;
					ny_error_set(&error, NY_ERROR_DOMAIN_ERRNO, errno);
					tcp->event_tcp_error(tcp, &error);
				}

				/* Close connection */
				safe_close(fd);
				break;
			}

			/* Initialise user-data pointer */
			conn->data = nil;

			/* Initialise timeout watcher */
			/* FIXME: Hard-coded timeout value */
			ev_timer_init(&conn->timer, timeout_event, 60.0, 60.0);
			conn->timer.data = conn;
			ev_timer_start(EV_A_ &conn->timer);

			/* Initialise I/O watcher */
			ev_io_init(&conn->io, conn_event, fd, EV_READ);
			conn->io.data = conn;
			ev_io_start(EV_A_ &conn->io);
		}
	}
}

int ny_tcp_init(struct ny_tcp *restrict tcp, struct ny *restrict ny, char const *restrict node, char const *restrict service) {
	assert(tcp);
	assert(ny);

	int _;
	int ret = 0;

	/* Initialise structure */
	tcp->data = nil;
	tcp->ny = ny;
	tcp->event_tcp_error = nil;
	tcp->event_tcp_connect = nil;
	tcp->event_conn_error = nil;
	tcp->event_conn_readable = nil;
	tcp->event_conn_writable = nil;
	tcp->event_conn_timeout = nil;

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

void ny_tcp_destroy(struct ny_tcp *restrict tcp) {
	assert(tcp);

	int _;

	/* Close socket */
	_ = safe_close(tcp->io.fd);

	assert(_);
}

int ny_tcp_listen(struct ny_tcp *restrict tcp) {
	assert(tcp);

	int ret = 0;

	/* Listen on socket */
	if (listen(tcp->io.fd, SOMAXCONN)) {
		ret = errno;
		goto exit;
	}

	ev_io_start(tcp->ny->loop, &tcp->io);

exit:
	return ret;
}

void ny_tcp_conn_destroy(struct ny_tcp_conn *restrict conn) {
	assert(conn);

	/* Call destroy event handler */
	if (conn->tcp->event_conn_destroy)
		conn->tcp->event_conn_destroy(conn);

	/* Stop watchers */
	ev_io_stop(conn->tcp->ny->loop, &conn->io);
	ev_timer_stop(conn->tcp->ny->loop, &conn->timer);

	/* Close socket */
	safe_close(conn->io.fd);

	/* Free structure */
	free(conn);
}

void ny_tcp_conn_touch(struct ny_tcp_conn *restrict conn) {
	assert(conn);

	/* Reset timeout timer */
	ev_timer_again(conn->tcp->ny->loop, &conn->timer);
}
