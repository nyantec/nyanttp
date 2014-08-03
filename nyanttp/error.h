/**
 * \file
 *
 * \brief Error structure
 */

#pragma once
#ifndef __ny_error__
#define __ny_error__

#if defined __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <nyanttp/nothrow.h>

/**
 * \brief Error domain
 */
enum ny_error_domain {
	NY_ERROR_DOMAIN_NY, /**< ny internal error */
	NY_ERROR_DOMAIN_ERRNO, /**< Error code as defined in errno.h */
	NY_ERROR_DOMAIN_GAI /**< getaddrinfo() error */
};

enum ny_error_nyan {
	NY_ERROR_VERSION,
	NY_ERROR_EVVER,
	NY_ERROR_EVINIT,
	NY_ERROR_EVWATCH
};

/**
 * \brief Error structure
 */
struct ny_error {
	char const *file; /**< File name */
	char const *func; /**< Function name */
	unsigned int line; /**< Line number */
	enum ny_error_domain domain; /**< Error domain */
	int code; /**< Error code */
};

/**
 * \brief Get error message string
 *
 * \param[in] error Error description structure
 *
 * \return Pointer to error description
 *
 * Get a zero-terminated message string describing the error specified by \p
 * error and return a pointer to it. The string may be stored in a
 * thread-local buffer and may be overwritten by a subsequent call to the
 * function in the same thread.
 */
extern char const *ny_error(struct ny_error const *restrict error) ny_nothrow;

/**
 * \brief Get error message string, reentrant version
 *
 * \param[in] error Error description structure
 * \param[in,out] buffer Buffer to store message string
 * \param[in] length Buffer capacity
 *
 * \return Pointer to error description
 *
 * Get a message string describing the error specified by \p error and return a
 * pointer to it. The buffer described by \p buffer and \p length \em may be
 * used to store the string.
 */
extern char const *ny_error_r(struct ny_error const *restrict error, char
		*restrict buffer, size_t length) ny_nothrow;

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
#define ny_error_set(error, domain, code) (void) ( \
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
