#pragma once
#ifndef __nyanttp_error__
#define __nyanttp_error__

#if defined __cplusplus
extern "C" {
#endif

/**
 * \brief Error domain
 */
enum nyanttp_error_domain {
	NYANTTP_ERROR_DOMAIN_NYAN, /**< nyanttp internal error */
	NYANTTP_ERROR_DOMAIN_ERRNO, /**< Error code as defined in errno.h */
	NYANTTP_ERROR_DOMAIN_GAI /**< getaddrinfo() error */
};

enum nyanttp_error_nyan {
	NYANTTP_ERROR_EVINIT,
};

struct nyanttp_error {
	enum nyanttp_error_domain domain;
	int code;
};

/**
 * \brief Get error message string
 *
 * \param error Error description structure
 *
 * \return Pointer to error description
 */
extern char const *nyanttp_error(struct nyanttp_error const *restrict error);

#if defined __cplusplus
}
#endif

#endif
