/**
 * \file
 *
 * \brief Branch prediction macros
 */

#pragma once
#ifndef __ny_expect__
#define __ny_expect__

/**
 * \def expect (expr, value)
 *
 * \brief Expect expression to yield certain value.
 *
 * \param expr  Expression.
 * \param value Expected value.
 *
 * \return Actual value.
 */

#if defined __GNUC__ || defined __clang__
#	define expect(expr, value) __builtin_expect ((expr), (value))
#else
#	define expect(expr, value) (expr)
#endif

/**
 * \brief Expect expression to yield non‐zero value.
 *
 * \param expr Expression.
 *
 * \return \c 1 if \a expr evaluates to non‐zero value or \c 0 otherwise.
 */
#define likely(expr) expect (!!(expr), 1)

/**
 * \brief Expect expression to yield zero value.
 *
 * \param expr Expression.
 *
 * \return \c 1 if \a expr evaluates to non‐zero value or \c 0 otherwise.
 */
#define unlikely(expr) expect (!!(expr), 0)

#endif
