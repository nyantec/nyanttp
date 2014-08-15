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
	char const *prio) {
	assert(tls);
	assert(ny);

	int _;
	int ret = -1;

	/* Initialise structure */
	tls->data = NULL;
	tls->ny = ny;

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
		goto exit;
	}

	_ = gnutls_dh_params_generate2(tls->dh_params, dh_bits);
	if (unlikely(_)) {
		ny_error_set(&tls->ny->error, NY_ERROR_DOMAIN_GTLS, _);
		goto exit;
	}

	ret = 0;

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
