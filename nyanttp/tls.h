#pragma once
#ifndef __ny_tls__
#define __ny_tls__

#if defined __cplusplus
extern "C" {
#endif

#include <gnutls/gnutls.h>

#include <nyanttp/ny.h>
#include <nyanttp/alloc.h>
#include <nyanttp/tcp.h>

struct ny_tls;
struct ny_tls_sess;

/**
 * \brief TLS listener context
 */
struct ny_tls {
	void *data; /**< User data */
	struct ny *ny; /**< Context structure */
	struct ny_alloc alloc_sess; /**< Session memory pool */
	gnutls_priority_t prio_cache;
	gnutls_dh_params_t dh_params; /**< Diffie‐Hellman parameters */

	void (*tls_error)(struct ny_tls *restrict,
		struct ny_error const *restrict);

	void (*sess_error)(struct ny_tls_sess *restrict,
		struct ny_error const *restrict);
	void (*sess_destroy)(struct ny_tls_sess *restrict);
	void (*sess_connect)(struct ny_tls_sess *restrict);
	void (*sess_readable)(struct ny_tls_sess *restrict);
	void (*sess_writable)(struct ny_tls_sess *restrict);
	bool (*sess_timeout)(struct ny_tls_sess *restrict);

	ssize_t (*trans_recv)(void *restrict, void *restrict, size_t);
	ssize_t (*trans_send_vec)(void *restrict,
		struct iovec const *restrict, size_t);
	void (*trans_event)(void *restrict, int);
	void (*trans_close)(void *restrict);
};

/**
 * \brief TLS session context
 */
struct ny_tls_sess {
	void *data; /**< User data */
	void *trans; /**< Transport data */
	struct ny_tls *tls; /**< TLS listener */
	gnutls_session_t session; /**< GnuTLS session */
	bool handshake;
};

/**
 * \brief Initialise TLS context
 *
 * \param[out] tls Pointer to TLS listener
 * \param[in,out] ny Pointer to context structure
 * \param[in] prio TLS cipher priority string
 */
extern int ny_tls_init(struct ny_tls *restrict tls, struct ny *restrict ny,
	char const *restrict prio, uint_least32_t maxsess);

/**
 * \brief Destroy TLS context
 *
 * \param[in,out] tls Pointer to TLS listener
 */
extern void ny_tls_destroy(struct ny_tls *restrict tls);

extern int ny_tls_listen(struct ny_tls *restrict tls);

extern void ny_tls_connect(struct ny_tls *restrict tls, void *restrict trans);

extern void ny_tls_sess_destroy(struct ny_tls_sess *restrict sess);

extern void ny_tls_sess_readable(struct ny_tls_sess *restrict sess);

extern void ny_tls_sess_writable(struct ny_tls_sess *restrict sess);

extern ssize_t ny_tls_sess_recv(struct ny_tls_sess *restrict sess,
	void *restrict buffer, size_t length);

extern ssize_t ny_tls_sess_send(struct ny_tls_sess *restrict sess,
	void const *restrict buffer, size_t length);

extern ssize_t ny_tls_sess_sendfile(struct ny_tls_sess *restrict sess,
	int fd, size_t length, off_t offset);

#if defined __cplusplus
}
#endif

#endif
