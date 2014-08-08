/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>

#include <nyanttp/expect.h>

static int fcntl_set(int fd, int get, int set, int flags) {
	int ret = fcntl(fd, get);
	if (unlikely(ret < 0))
		goto exit;

	ret = fcntl(fd, set, ret | flags);

exit:
	return ret;
}

int ny_io_fd_set(int fd, int flags) {
	return fcntl_set(fd, F_GETFD, F_SETFD, flags);
}

int ny_io_fl_set(int fd, int flags) {
	return fcntl_set(fd, F_GETFL, F_SETFL, flags);
}

int ny_io_open(char const *restrict path, int flags) {
	int fd;

	do {
		fd =
#if defined(O_CLOEXEC)
			open(path, flags | O_NONBLOCK | O_CLOEXEC);
#else
			open(path, flags | O_NONBLOCK);
#endif
	} while (unlikely(fd < 0 && errno == EINTR));

#if !defined(O_CLOEXEC)
	int _ = ny_io_fd_set(fd, FD_CLOEXEC);
	assert(!_);
#endif

	return fd;
}

int ny_io_close(int fd) {
	int ret;

	do {
		ret = close(fd);
	} while (unlikely(ret && errno == EINTR));

	return ret;
}

ssize_t ny_io_read(int fd, void *restrict buffer, size_t length) {
	ssize_t rlen;

	do {
		rlen = read(fd, buffer, length);
	} while (unlikely(rlen < 0 && errno == EINTR));

	return rlen;
}

ssize_t ny_io_pread(int fd, void *restrict buffer, size_t length, off_t offset) {
	ssize_t rlen;

	do {
		rlen = pread(fd, buffer, length, offset);
	} while (unlikely(rlen < 0 && errno == EINTR));

	return rlen;
}

ssize_t ny_io_write(int fd, void const *restrict buffer, size_t length) {
	ssize_t wlen;

	do {
		wlen = write(fd, buffer, length);
	} while (unlikely(wlen < 0 && errno == EINTR));

	return wlen;
}

ssize_t ny_io_pwrite(int fd, void const *restrict buffer, size_t length,
	off_t offset) {
	ssize_t wlen;

	do {
		wlen = pwrite(fd, buffer, length, offset);
	} while (unlikely(wlen < 0 && errno == EINTR));

	return wlen;
}

int ny_io_accept(int lsock, struct sockaddr *restrict address,
	socklen_t *restrict addrlen) {
	int csock;

	do {
		csock =
#if HAVE_ACCEPT4
			accept4(lsock, address, addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
			accept(lsock, address, addrlen);
#endif
	} while (unlikely(csock < 0 && errno == EINTR));

#if !HAVE_ACCEPT4
	if (likely(csock >= 0)) {
		int _;
		_ = ny_io_fl_set(csock, O_NONBLOCK);
		assert(!_);

		_ = ny_io_fd_set(csock, FD_CLOEXEC);
		assert(!_);
	}
#endif

	return csock;
}

void *ny_io_mmap_ro(int fd, size_t length, off_t offset) {
	void *memory = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, offset);
	if (unlikely(memory == MAP_FAILED))
		memory = NULL;

	return memory;
}

ssize_t ny_io_sendfile(int out, int in, size_t length, off_t offset) {
	ssize_t wlen = -1;

#if NY_IO_SENDFILE_LINUX
	/* Linux‐style sendfile */
	wlen = sendfile(out, in, &offset, length);
#elif NY_IO_SENDFILE_BSD
	/* BSD‐style sendfile */
	off_t sbytes;
	int _ = sendfile(out, in, offset, length, nil, &sbytes, 0);
	if (likely(!_))
		wlen = sbytes;
#else
	/* Emulate sendfile using mmap and write */
	void *memory = ny_io_mmap_ro(in, length, offset);
	if (unlikely(!memory))
		goto exit;

	wlen = ny_io_write(out, memory, length);

	int _ = munmap(memory, length);
	assert(!_);
#endif

exit:
	return wlen;
}
