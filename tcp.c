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

#if NY_TCP_SENDFILE_LINUX
#	include <sys/sendfile.h>
#elif NY_TCP_SENDFILE_BSD
#	include <sys/uio.h>
#elif NY_TCP_SENDFILE_MMAP
#	include <sys/mman.h>
#endif

#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include <nyanttp/ny.h>
#include <nyanttp/tcp.h>

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

static void fcntl_set(int fd, int get, int set, int flags) {
	int _;

	_ = fcntl(fd, get);
	assert(_ >= 0);
	_ = fcntl(fd, set, _ | flags);
	assert(_);
}

static void ny_fd_set(int fd, int flags) {
	fcntl_set(fd, F_GETFD, F_SETFD, flags);
}

static void ny_fl_set(int fd, int flags) {
	fcntl_set(fd, F_GETFL, F_SETFL, flags);
}

static int goat_new() {
	int goat = fcntl(0, F_DUPFD, 0);
	assert(goat >= 0);

	/* Close on exec */
	ny_fd_set(goat, FD_CLOEXEC);

	return goat;
}

/**
 * \brief Close file descriptor, retrying if interrupted
 *
 * \param[in] fd File descriptor to close
 *
 * \return Zero on success or non-zero on failure
 */
static int close_retry(int fd) {
	int ret;

	do {
		ret = close(fd);
	} while (ret && errno == EINTR);

	return ret;
}

/**
 * \brief Accept new connection, setting flags
 *
 * \param[in] lsock File descriptor of the listening socket
 * \param[out] address Pointer to a \c sockaddr structure to hold the address of the connecting socket
 * \param[in,out] addrlen Capacity of the structure given by \p address on input, the actual length of the stored address on output
 *
 * \return Non-negative file descriptor of the accepted socket or a negative integer on failure
 */
static int accept_flags(int lsock, struct sockaddr *restrict address,
	socklen_t *restrict addrlen) {
	int csock =
#if HAVE_ACCEPT4
		accept4(lsock, address, addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
		accept(lsock, address, addrlen);

	if (likely(csock >= 0)) {
		/* Close socket on exec */
		ny_fd_set(fd, FD_CLOEXEC);

		/* Mark socket as non-blocking */
		ny_fl_set(fd, O_NONBLOCK);
	}
#endif

	return csock;
}

/**
 * \brief Accept new connection, retrying if interrupted
 *
 * \param[in] lsock File descriptor of the listening socket
 * \param[out] address Pointer to a \c sockaddr structure to hold the address of the connecting socket
 * \param[in,out] addrlen Capacity of the structure given by \p address on input, the actual length of the stored address on output
 *
 * \return Non-negative file descriptor of the accepted socket or a negative integer on failure
 */
static int accept_retry(int lsock, struct sockaddr *restrict address,
	socklen_t *restrict addrlen) {
	int csock;

	do {
		csock = accept_flags(lsock, address, addrlen);
	} while (unlikely(csock < 0 && errno == EINTR));

	return csock;
}

static int accept_safe(struct ny_tcp *restrict tcp,
	struct sockaddr *restrict address, socklen_t *restrict addrlen) {
	int _;

	int csock = accept_retry(tcp->io.fd, address, addrlen);

	/* Reject connection if out of file descriptors */
	if (unlikely(csock < 0 && (errno == EMFILE || errno == ENFILE))) {
		int save = errno;
		_ = close_retry(tcp->goat);
		assert(!_);

		csock = accept_retry(tcp->io.fd, address, addrlen);
		if (likely(csock >= 0))
			close_retry(csock);

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
					close_retry(fd);
					continue;
				}
			}

			/* Allocate memory for connection structure */
			struct ny_tcp_con *con = ny_alloc_acquire(&tcp->alloc_con);
			if (unlikely(!con)) {
				if (tcp->tcp_error)
					tcp->tcp_error(tcp, &tcp->ny->error);

				/* Close connection */
				close_retry(fd);
				break;
			}

			/* Initialise user-data pointer */
			con->data = nil;

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
	tcp->data = nil;
	tcp->ny = ny;
	tcp->tcp_error = nil;
	tcp->tcp_connect = nil;
	tcp->con_error = nil;
	tcp->con_readable = nil;
	tcp->con_writable = nil;
	tcp->con_timeout = nil;

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
		if (unlikely(fd < 0))
			continue;

		/* Bind to socket */
		if (likely(!bind(fd, rp->ai_addr, rp->ai_addrlen)))
			break;

		close_retry(fd);
	}

	if (fd < 0) {
		/* FIXME: Define error code */
		goto exit;
	}

	/* Close socket on exec */
	ny_fd_set(fd, FD_CLOEXEC);

	/* Mark socket as non-blocking */
	ny_fl_set(fd, O_NONBLOCK);

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

exit:
	if (likely(res))
		freeaddrinfo(res);

	return ret;
}

void ny_tcp_destroy(struct ny_tcp *restrict tcp) {
	assert(tcp);

	int _;

	/* Close socket */
	_ = close_retry(tcp->io.fd);
	assert(_);

	/* Destroy allocator */
	ny_alloc_destroy(&tcp->alloc_con);

	/* Close reserve file descriptor */
	_ = close_retry(tcp->goat);
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

void ny_tcp_con_destroy(struct ny_tcp_con *restrict con) {
	assert(con);

	/* Call destroy event handler */
	if (con->tcp->con_destroy)
		con->tcp->con_destroy(con);

	/* Stop watchers */
	ev_io_stop(con->tcp->ny->loop, &con->io);
	ev_timer_stop(con->tcp->ny->loop, &con->timer);

	/* Close socket */
	close_retry(con->io.fd);

	/* Free structure */
	free(con);
}

void ny_tcp_con_touch(struct ny_tcp_con *restrict con) {
	assert(con);

	/* Reset timeout timer */
	ev_timer_again(con->tcp->ny->loop, &con->timer);
}

ssize_t ny_tcp_con_recv(struct ny_tcp_con *restrict con,
	void *restrict buffer, size_t length) {
	assert(con);
	assert(buffer);

	ssize_t rlen;

	do {
		rlen = read(con->io.fd, buffer, length);
	} while (unlikely(rlen < 0 && errno == EINTR));

	if (unlikely(rlen < 0))
		ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);

	/* Reset timeout */
	else if (likely(rlen > 0))
		ny_tcp_con_touch(con);

	return rlen;
}

ssize_t ny_tcp_con_send(struct ny_tcp_con *restrict con,
	void const *restrict buffer, size_t length) {
	assert(con);
	assert(buffer);

	ssize_t wlen;

	do {
		wlen = write(con->io.fd, buffer, length);
	} while (unlikely(wlen < 0 && errno == EINTR));

	if (unlikely(wlen < 0))
		ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);

	/* Reset timeout */
	else if (likely(wlen > 0))
		ny_tcp_con_touch(con);

	return wlen;
}

ssize_t ny_tcp_con_sendfile(struct ny_tcp_con *restrict con,
	int fd, off_t offset, size_t length) {
	assert(con);
	assert(fd >= 0);

	int _;
	ssize_t wlen = -1;

#if NY_TCP_SENDFILE_LINUX
	wlen = sendfile(con->io.fd, fd, &offset, length);
#elif NY_TCP_SENDFILE_BSD
	off_t sbytes;
	_ = sendfile(con->io.fd, fd, offset, length, nil, &sbytes, 0);
	if (likely(!_))
		wlen = sbytes;
#else
#	if NY_TCP_SENDFILE_MMAP
	/* Emulate sendfile with mmap and write */
	void *buffer = mmap(nil, length, PROT_READ, MAP_SHARED, fd, offset);
	if (unlikely(map == MAP_FAILED)) {
		ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		goto exit;
	}

	posix_madvise(buffer, length, POSIX_MADV_SEQUENTIAL | POSIX_MADV_WILLNEED);
#	else
	/* Emulate sendfile with read and write */
	ssize_t rlen;

	/* Limit buffer size */
	if (length > 1024)
		length = 1024;

	/* FIXME: Place the buffer somewhere else */
	uint8_t buffer[length];

	do {
		rlen = pread(fd, buffer, length, offset);
	} while (unlikely(rlen < 0 && errno == EINTR));

	if (unlikely(rlen < 0)) {
		ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		goto exit;
	}

	length = rlen;
#	endif

	do {
		wlen = write(con->io.fd, buffer, length);
	} while (unlikely(wlen < 0 && errno == EINTR));
#endif

	if (unlikely(wlen < 0)) {
		ny_error_set(&con->tcp->ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		goto unmap;
	}

	/* Reset timeout */
	ny_tcp_con_touch(con);

unmap:;
#if NY_TCP_SENDFILE_MMAP
	_ = munmap(buffer, length);
	assert(!_);
#endif

exit:
	return wlen;
}
