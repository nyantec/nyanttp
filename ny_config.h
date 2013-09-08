#if HAVE_FORK
#	define NY_MPROC 1
#endif

#if HAVE_MMAP && (HAVE_DECL_MAP_ANONYMOUS || HAVE_DECL_MAP_ANON)
#	define NY_ALLOC_MMAP 1
#	define NY_ALLOC_ALIGN 1

	/* Define MAP_ANONYMOUS if necessary */
#	if !HAVE_DECL_MAP_ANONYMOUS
#		define MAP_ANONYMOUS MAP_ANON
#	endif

#	if HAVE_MPROTECT
#		define NY_ALLOC_GUARD 1
#	endif

#	if HAVE_POSIX_MADVISE
#		define NY_ALLOC_ADVISE 1
#	endif
#endif