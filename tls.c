/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <assert.h>
#include <errno.h>

#include <nyanttp/tls.h>
#include <nyanttp/expect.h>

int ny_tls_init(struct ny_tls *restrict tls, struct ny *restrict ny,
	char const *prio, uint_least32_t maxsess) {
	assert(tls);
	assert(ny);

	int _;
	int ret = -1;

	/* Initialise structure */
	tls->data = NULL;
	tls->ny = ny;
	tls->tls_error = NULL;
	tls->sess_error = NULL;
	tls->sess_destroy = NULL;
	tls->sess_readable = NULL;
	tls->sess_writable = NULL;

	tls->trans_recv = NULL;
	tls->trans_send_vec = NULL;
	tls->trans_close = NULL;

	/* Initialise GnuTLS */
	gnutls_global_init();

	char const *err_pos;
	_ = gnutls_priority_init(&tls->prio_cache,
		prio ? prio : NY_TLS_DEFAULT_PRIO, &err_pos);
	if (unlikely(_)) {
		ny_error_set(&tls->ny->error, NY_ERROR_DOMAIN_GTLS, _);
		goto exit;
	}

	/* Generate Diffie‐Hellman parameters */
	unsigned int dh_bits = gnutls_sec_param_to_pk_bits(GNUTLS_PK_DH,
		GNUTLS_SEC_PARAM_HIGH);

	_ = gnutls_dh_params_init(&tls->dh_params);
	if (unlikely(_)) {
		ny_error_set(&tls->ny->error, NY_ERROR_DOMAIN_GTLS, _);
		goto deinit_gtls;
	}

	_ = gnutls_dh_params_generate2(tls->dh_params, dh_bits);
	if (unlikely(_)) {
		ny_error_set(&tls->ny->error, NY_ERROR_DOMAIN_GTLS, _);
		goto exit;
	}

	/* Initialise allocator */
	_ = ny_alloc_init(&tls->alloc_sess, ny, maxsess,
		sizeof (struct ny_tls_sess));
	if (unlikely(_))
		goto deinit_dh;

	ret = 0;
	goto exit;

deinit_dh:
	/* Deinitialise Diffie‐Hellman parameters */
	gnutls_dh_params_deinit(tls->dh_params);

deinit_gtls:
	/* Deinitialise GnuTLS */
	gnutls_global_deinit();

exit:
	return ret;
}

void ny_tls_destroy(struct ny_tls *restrict tls) {
	assert(tls);

	/* Deinitialise Diffie‐Hellman parameters */
	gnutls_dh_params_deinit(tls->dh_params);

	/* Deinitialise GnuTLS */
	gnutls_global_deinit();
}

static ssize_t gtls_recv(struct ny_tls_sess *restrict sess,
	void *restrict buffer, size_t length) {
	assert(sess);

	ssize_t rlen = sess->tls->trans_recv(sess->trans, buffer, length);
	if (unlikely(rlen == 0)) {
		rlen = -1;
		gnutls_transport_set_errno(sess->session, EAGAIN);
	}
	else if (unlikely(rlen < 0)) {
		gnutls_transport_set_errno(sess->session,
			likely(sess->tls->ny->error.domain == NY_ERROR_DOMAIN_ERRNO) ?
				sess->tls->ny->error.code : EINVAL );
	}

	return rlen;
}

static ssize_t gtls_send_vec(struct ny_tls_sess *restrict sess,
	struct iovec const *restrict vector, size_t count) {
	assert(sess);
	assert(vector);

	ssize_t wlen = sess->tls->trans_send_vec(sess->trans, vector, count);
	if (unlikely(wlen == 0)) {
		wlen = -1;
		gnutls_transport_set_errno(sess->session, EAGAIN);
	}
	else if (unlikely(wlen < 0)) {
		gnutls_transport_set_errno(sess->session,
			likely(sess->tls->ny->error.domain == NY_ERROR_DOMAIN_ERRNO) ?
				sess->tls->ny->error.code : EINVAL );
	}

	return wlen;
}

void ny_tls_connect(struct ny_tls *restrict tls, void *restrict trans) {
	assert(tls);

	int _;

	struct ny_tls_sess *sess = ny_alloc_acquire(&tls->alloc_sess);
	if (unlikely(!sess)) {
		if (tls->tls_error)
			tls->tls_error(tls, &tls->ny->error);
		goto close;
	}

	sess->data = NULL;
	sess->trans = trans;
	sess->tls = tls;

	_ = gnutls_init(&sess->session, GNUTLS_SERVER);
	if (unlikely(_)) {
		if (tls->tls_error) {
			struct ny_error error;
			ny_error_set(&error, NY_ERROR_DOMAIN_GTLS, _);
			tls->tls_error(tls, &error);
		}

		goto close;
	}

	_ = gnutls_priority_set(sess->session, tls->prio_cache);
	if (unlikely(_)) {
		if (tls->tls_error) {
			struct ny_error error;
			ny_error_set(&error, NY_ERROR_DOMAIN_GTLS, _);
			tls->tls_error(tls, &error);
		}

		goto deinit;
	}

	/* TODO: Credentials */

	/* Setup transport layer */
	gnutls_transport_set_ptr(sess->session, sess);
	gnutls_transport_set_pull_function(sess->session,
		(gnutls_pull_func) gtls_recv);
	gnutls_transport_set_vec_push_function(sess->session,
		(gnutls_vec_push_func) gtls_send_vec);

	goto exit;

deinit:
	gnutls_deinit(sess->session);

close:
	tls->trans_close(trans);

release:
	ny_alloc_release(&tls->alloc_sess, sess);

exit:
	return;
}

static void gtls_handshake(struct ny_tls_sess *restrict sess) {
	int _;

	if (!sess->handshake) {
		_ = gnutls_handshake(sess->session);
		if (unlikely(_)) {
			if (likely(_ == GNUTLS_E_AGAIN)) {
				/* FIXME: Define independent event codes */
				sess->tls->trans_event(sess->trans,
					gnutls_record_get_direction(sess->session) ?
						NY_TCP_WRITABLE : NY_TCP_READABLE);
			}
			else if (sess->tls->sess_error) {
				struct ny_error error;
				ny_error_set(&error, NY_ERROR_DOMAIN_GTLS, _);
				sess->tls->sess_error(sess, &error);
			}

			return;
		}

		/* FIXME: Reset events */
		sess->handshake = true;
	}
}

void ny_tls_sess_readable(struct ny_tls_sess *restrict sess) {
	assert(sess);

	gtls_handshake(sess);

	if (sess->tls->sess_readable)
		sess->tls->sess_readable(sess);
}

void ny_tls_sess_writable(struct ny_tls_sess *restrict sess) {
	assert (sess);

	gtls_handshake(sess);

	if (sess->tls->sess_writable)
		sess->tls->sess_writable(sess);
}

void ny_tls_sess_destroy(struct ny_tls_sess *restrict sess) {
	assert(sess);
	assert(sess->tls->trans_close);

	if (sess->tls->sess_destroy)
		sess->tls->sess_destroy(sess);

	sess->tls->trans_close(sess->trans);

	/* Free session structure */
	ny_alloc_release(&sess->tls->alloc_sess, sess);
}

ssize_t ny_tls_sess_recv(struct ny_tls_sess *restrict sess, 
	void *restrict buffer, size_t length) {
	assert(sess);
	assert(buffer);

	ssize_t rlen = gnutls_record_recv(sess->session, buffer, length);
	if (unlikely(rlen == 0)) {
		rlen = -1;
		ny_error_set(&sess->tls->ny->error, NY_ERROR_DOMAIN_NY, NY_ERROR_EOF);
	}
	else if (unlikely(rlen < 0)) {
		if (rlen == GNUTLS_E_AGAIN)
			rlen = 0;
		else if (rlen == GNUTLS_E_REHANDSHAKE) {
			sess->handshake = false;
			gtls_handshake(sess);
		}
		else
			ny_error_set(&sess->tls->ny->error, NY_ERROR_DOMAIN_GTLS, rlen);
	}

	return rlen;
}

ssize_t ny_tls_sess_send(struct ny_tls_sess *restrict sess,
	void const *restrict buffer, size_t length) {
	assert(sess);
	assert(buffer);

	ssize_t wlen = gnutls_record_send(sess->session, buffer, length);
	if (unlikely(wlen < 0)) {
		if (wlen == GNUTLS_E_AGAIN)
			wlen = 0;
		else if (wlen == GNUTLS_E_REHANDSHAKE) {
			sess->handshake = false;
			gtls_handshake(sess);
		}
		else
			ny_error_set(&sess->tls->ny->error, NY_ERROR_DOMAIN_GTLS, wlen);
	}

	return wlen;
}
