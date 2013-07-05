CPPFLAGS ?= -D_FORTIFY_SOURCE=2 -pedantic -Wall
CFLAGS   ?= -pipe -O2 -fstack-protector
PREFIX   ?= usr/local

CPPFLAGS += -D_XOPEN_SOURCE=700 -std=c11 -pthread
LDFLAGS  += -lev

libnyanttp: libnyanttp.a(nyanttp.o) libnyanttp.a(http.o)

.c.a:
	@set -e; \
	echo '  CC $<'; \
	trap 'rm -f $*.o' EXIT; \
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< $(LDFLAGS); \
	$(AR) -r -c $@ $*.o

.SUFFIXES: .a .c
