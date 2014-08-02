/**
 * \file
 *
 * \brief pure attribute
 */

#pragma once
#ifndef __ny_pure__
#define __ny_pure__

/**
 * \def ny_pure
 *
 * \brief Pure function
 *
 * A function declared pure has no side effects and the return value depends
 * solely on the parameters and global memory.
 */

#if defined __clang__ || defined __GNUC__
#	define ny_pure __attribute__ ((pure))
#else
#	define ny_pure
#endif

#endif
