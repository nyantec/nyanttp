@INC_AMINCLUDE@

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libny.la
libny_la_SOURCES = ny_config.h ny.c error.c urldecode.c urlencode.c util.c mem.c io.c alloc.c tcp.c tls.c http.c
libny_la_CPPFLAGS = $(AM_CPPFLAGS) $(libev_CFLAGS) $(GnuTLS_CFLAGS)
libny_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(NY_VERSION_LIBVER)
libny_la_LIBADD = $(libev_LIBS) $(GnuTLS_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = nyanttp.pc

EXTRA_DIST = Licence

SUBDIRS = nyanttp test
