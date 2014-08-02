/**
 * \file
 *
 * \brief const attribute
 */

#pragma once
#ifndef __ny_const__
#define __ny_const__

/**
 * \def ny_const
 *
 * \brief Constant function
 *
 * A function declared const has no side effects and its return value depends
 * solely on the parameters. It must not read global memory.
 */

#if defined __clang__ || defined __GNUC__
#	define ny_const __attribute__ ((const))
#else
#	define ny_const
#endif

#endif
