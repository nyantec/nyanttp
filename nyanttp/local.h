/**
 * \file
 *
 * \brief Thread‐local storage
 */

#pragma once
#ifndef __ny_local__
#define __ny_local__

/**
 * \def ny_local
 *
 * \brief Thread‐local storage class
 */

#if __STDC_VERSION__ >= 201000L && !defined __STDC_NO_THREADS__
#	define ny_local _Thread_local
#elif defined __GNUC__ || defined __clang__
#	define ny_local __thread
#else
#	error "No thread‐local storage implementation"
#endif

#endif
