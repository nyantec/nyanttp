/**
 * \file
 *
 * \brief I/O helper functions
 */

#pragma once
#ifndef __ny_io__
#define __ny_io__

#if defined __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <sys/uio.h>

/**
 * \brief Set file descriptor flags
 *
 * \param[in] fd File descriptor
 * \param[in] flags Flags to set
 *
 * \return Zero on success or non-zero on error
 */
extern int ny_io_fd_set(int fd, int flags);

/**
 * \brief Set file status flags
 *
 * \param[in] fd File descriptor
 * \param[in] flags Flags to set
 *
 * \return Zero on success or non-zero on error
 */
extern int ny_io_fl_set(int fd, int flags);

/**
 * \brief Open a file
 *
 * \param[in] path File path
 * \param[in] flags File open flags
 *
 * \return Non-negative file descriptor or a negative integer on failure
 */
extern int ny_io_open(char const *restrict path, int flags);

/**
 * \brief Close a file descriptor
 *
 * \brief[in] fd File descriptior
 *
 * \return Zero on success or non-zero on error
 */
extern int ny_io_close(int fd);

extern ssize_t ny_io_read(int fd, void *restrict buffer, size_t length);
extern ssize_t ny_io_readv(int fd, struct iovec const *restrict vector,
	size_t count);
extern ssize_t ny_io_pread(int fd, void *restrict buffer, size_t length,
	off_t offset);

extern ssize_t ny_io_write(int fd, void const *restrict buffer, size_t length);
extern ssize_t ny_io_writev(int fd, struct iovec const *restrict vector,
	size_t count);
extern ssize_t ny_io_pwrite(int fd, void const *restrict buffer, size_t length,
	off_t offset);

extern int ny_io_accept(int lsock, struct sockaddr *restrict address,
	socklen_t *restrict addrlen);

extern void *ny_io_mmap_ro(int fd, size_t length, off_t offset);

extern ssize_t ny_io_sendfile(int out, int in, size_t length, off_t offset);

#if defined __cplusplus
}
#endif

#endif
