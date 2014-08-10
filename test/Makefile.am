@INC_AMINCLUDE@

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)
LDADD = $(top_builddir)/libny.la

check_PROGRAMS = \
	ny_version ny_init ny_destroy ny_run \
	ny_error_set ny_error_unknown ny_error_errno \
	ny_alloc_init ny_alloc_destroy ny_alloc_overlap ny_alloc_linear ny_alloc_random \
	ny_urldecode_valid ny_urldecode_invalid ny_urlencode_valid ny_urlencode_invalid \
	ny_urlencode_urldecode

TESTS = $(check_PROGRAMS)
