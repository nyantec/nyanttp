@INC_AMINCLUDE@

lib_LTLIBRARIES = libny.la
libny_la_SOURCES = ny_config.h ny.c error.c urldecode.c urlencode.c util.c mem.c io.c alloc.c tcp.c http.c
libny_la_CPPFLAGS = $(AM_CPPFLAGS) $(libev_CFLAGS) $(OPENSSL_INCLUDES)
libny_la_LDFLAGS = $(AM_LDFLAGS) $(OPENSSL_LDFLAGS) -version-info $(NY_VERSION_LIBVER)
libny_la_LIBADD = $(libev_LIBS) $(OPENSSL_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = nyanttp.pc

EXTRA_DIST = Licence

SUBDIRS = nyanttp test
