/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <assert.h>

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
	tls->sess_recv = NULL;
	tls->sess_send = NULL;
	tls->sess_close = NULL;

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

void ny_tls_sess_destroy(struct ny_tls_sess *restrict sess) {
	assert(sess);
	assert(sess->tls->sess_close);

	if (sess->tls->sess_destroy)
		sess->tls->sess_destroy(sess);

	sess->tls->sess_close(sess);

	/* Free session structure */
	ny_alloc_release(&sess->tls->alloc_sess, sess);
}
