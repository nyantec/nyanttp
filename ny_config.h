#if HAVE_FORK
#	define NY_MPROC 1
#endif

/* Define MAP_ANONYMOUS if necessary */
#if !HAVE_DECL_MAP_ANONYMOUS
#	define MAP_ANONYMOUS MAP_ANON
#endif

#if HAVE_MPROTECT
#	define NY_MEM_ALLOC_GUARD 1
#endif

#if HAVE_POSIX_MADVISE
#	define NY_ALLOC_ADVISE 1
#endif

/* Maximum number of TCP connections to accept per event */
#define NY_TCP_ACCEPT_MAX 16

/* TCP connection activity timeout */
#define NY_TCP_TIMEOUT 60.0

/* TCP accept event priority */
#define NY_TCP_ACCEPT_PRIO EV_MINPRI

/* TCP timeout event priority */
#define NY_TCP_TIMER_PRIO (EV_MINPRI / 2)

/* TCP I/O event priority */
#define NY_TCP_IO_PRIO 0

/* TLS default cipher priorities */
#define NY_TLS_DEFAULT_PRIO "PFS:-3DES-CBC:-ARCFOUR-128:-SHA1:+COMP-DEFLATE:-VERS-SSL3.0:-VERS-TLS1.0:-VERS-DTLS1.0:-SIGN-RSA-SHA1:-SIGN-DSA-SHA1:-SIGN-ECDSA-SHA1:%LATEST_RECORD_VERSION:%SAFE_RENEGOTIATION:%STATELESS_COMPRESSION"
