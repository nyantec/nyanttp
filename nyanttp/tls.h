#pragma once
#ifndef __ny_tls__
#define __ny_tls__

#if defined __cplusplus
extern "C" {
#endif

#include <gnutls/gnutls.h>

#include <nyanttp/ny.h>

struct ny_tls {
	void *data; /**< User data */
	struct ny *ny; /**< Context structure */
	gnutls_priority_t prio_cache;
	gnutls_dh_params_t dh_params; /**< Diffie‐Hellman parameters */
};



#if defined __cplusplus
}
#endif

#endif
