@INC_AMINCLUDE@

lib_LTLIBRARIES = libny.la
libny_la_SOURCES = ny.c error.c alloc.c tcp.c
libny_la_CFLAGS = $(AM_CFLAGS) $(libev_CFLAGS)
libny_la_LDFLAGS = $(AM_LDFLAGS) $(libev_LIBS)

EXTRA_DIST = Licence

SUBDIRS = ny test
