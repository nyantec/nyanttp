@INC_AMINCLUDE@

lib_LTLIBRARIES = libny.la
libny_la_SOURCES = \
	ny.c error.c alloc.c tcp.c \
	ny.h error.h alloc.h tcp.h
libny_la_CFLAGS = $(AM_CFLAGS) $(libev_CFLAGS)
libny_la_LDFLAGS = $(AM_LDFLAGS) $(libev_LIBS)

check_PROGRAMS = testny
testny_SOURCES = test.c
testny_LDADD = libny.la
testny_LDFLAGS = $(AM_LDFLAGS) -lcheck

TESTS = $(check_PROGRAMS)
