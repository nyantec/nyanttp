/**
 * \file
 *
 * \brief nothrow attribute
 */

#pragma once
#ifndef __ny_nothrow__
#define __ny_nothrow__

/**
 * \def ny_nothrow
 *
 * \brief Function does not throw exceptions
 */
#if defined __cplusplus
#	if __cplusplus > 199711l
#		define ny_nothrow noexcept
#	else
#		define ny_nothrow throw()
#	endif
#elif defined __clang__ || defined __GNUC__
#	define ny_nothrow __attribute__ ((nothrow))
#else
#	define ny_nothrow
#endif

#endif
