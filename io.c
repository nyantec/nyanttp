/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>

#if NY_TCP_SENDFILE_LINUX
#	include <sys/sendfile.h>
#elif NY_TCP_SENDFILE_BSD
#	include <sys/uio.h>
#endif

#include <nyanttp/expect.h>

static int fcntl_set(int fd, int get, int set, int flags) {
	assert(fd >= 0);

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
	assert(path);

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
	assert(fd >= 0);

	int ret;

	do {
		ret = close(fd);
	} while (unlikely(ret && errno == EINTR));

	return ret;
}

ssize_t ny_io_read(int fd, void *restrict buffer, size_t length) {
	assert(fd >= 0);
	assert(buffer);
	assert(length <= SSIZE_MAX);

	ssize_t rlen;

	do {
		rlen = read(fd, buffer, length);
	} while (unlikely(rlen < 0 && errno == EINTR));

	return rlen;
}

ssize_t ny_io_readv(int fd, struct iovec const *restrict vector, size_t count) {
	assert(fd >= 0);
	assert(vector);
	assert(count <= IOV_MAX);

	ssize_t rlen;

	do {
		rlen = readv(fd, vector, count);
	} while (unlikely(rlen < 0 && errno == EINTR));

	return rlen;
}

ssize_t ny_io_pread(int fd, void *restrict buffer, size_t length, off_t offset) {
	assert(fd >= 0);
	assert(buffer);
	assert(length <= SSIZE_MAX);
	assert(offset >= 0);

	ssize_t rlen;

	do {
		rlen = pread(fd, buffer, length, offset);
	} while (unlikely(rlen < 0 && errno == EINTR));

	return rlen;
}

ssize_t ny_io_write(int fd, void const *restrict buffer, size_t length) {
	assert(fd >= 0);
	assert(buffer);
	assert(length <= SSIZE_MAX);

	ssize_t wlen;

	do {
		wlen = write(fd, buffer, length);
	} while (unlikely(wlen < 0 && errno == EINTR));

	return wlen;
}

ssize_t ny_io_writev(int fd, struct iovec const *restrict vector, size_t count) {
	assert(fd >= 0);
	assert(vector);
	assert(count <= IOV_MAX);

	ssize_t wlen;

	do {
		wlen = writev(fd, vector, count);
	} while (unlikely(wlen < 0 && errno == EINTR));

	return wlen;
}

ssize_t ny_io_pwrite(int fd, void const *restrict buffer, size_t length,
	off_t offset) {
	assert(fd >= 0);
	assert(buffer);
	assert(length <= SSIZE_MAX);
	assert(offset >= 0);

	ssize_t wlen;

	do {
		wlen = pwrite(fd, buffer, length, offset);
	} while (unlikely(wlen < 0 && errno == EINTR));

	return wlen;
}

int ny_io_accept(int lsock, struct sockaddr *restrict address,
	socklen_t *restrict addrlen) {
	assert(lsock >= 0);
	assert(addrlen && (*addrlen >= 0));

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
	assert(fd >= 0);
	assert(length > 0);
	assert(offset >= 0);

	void *memory = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, offset);
	if (unlikely(memory == MAP_FAILED))
		memory = NULL;

	return memory;
}

ssize_t ny_io_sendfile(int out, int in, size_t length, off_t offset) {
	assert(out >= 0);
	assert(in >= 0);
	assert(length <= SSIZE_MAX);
	assert(offset >= 0);

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
