/**
 * \file
 *
 * \brief Error structure
 */

#pragma once
#ifndef __nyanttp_error__
#define __nyanttp_error__

#if defined __cplusplus
extern "C" {
#endif

#include <defy/nothrow>

/**
 * \brief Error domain
 */
enum nyanttp_error_domain {
	NYANTTP_ERROR_DOMAIN_NYAN, /**< nyanttp internal error */
	NYANTTP_ERROR_DOMAIN_ERRNO, /**< Error code as defined in errno.h */
	NYANTTP_ERROR_DOMAIN_GAI /**< getaddrinfo() error */
};

enum nyanttp_error_nyan {
	NYANTTP_ERROR_VERSION,
	NYANTTP_ERROR_EVVER,
	NYANTTP_ERROR_EVINIT,
	NYANTTP_ERROR_EVWATCH
};

/**
 * \brief Error structure
 */
struct nyanttp_error {
	char const *file; /**< File name */
	char const *func; /**< Function name */
	unsigned int line; /**< Line number */
	enum nyanttp_error_domain domain; /**< Error domain */
	int code; /**< Error code */
};

/**
 * \brief Get error message string
 *
 * \param[in] error Error description structure
 *
 * \return Pointer to error description
 */
extern char const *nyanttp_error(struct nyanttp_error const *restrict error) nothrow;

/**
 * \internal
 * \brief Set error structure parameters
 *
 * \param[out] error Error structure
 * \param[in] domain Error domain
 * \param[in] code Error code
 *
 * Set error structure parameters, automatically determining file name, function
 * and line number.
 */
#define nyanttp_error_set(error, domain, code) (void) ( \
	(error)->file = __FILE__, \
	(error)->func = __func__, \
	(error)->line = __LINE__, \
	(error)->d##omain = (domain), \
	(error)->c##ode = (code) \
)

#if defined __cplusplus
}
#endif

#endif
