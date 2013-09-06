CPPFLAGS ?= -D_FORTIFY_SOURCE=2 -pedantic -Wall -Wstrict-overflow=5 \
-Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn
CFLAGS   ?= -pipe -O2 -fstack-protector
PREFIX   ?= usr/local

CPPFLAGS += -D_XOPEN_SOURCE=700 -D_BSD_SOURCE -std=c11 -pthread
LDFLAGS  += -lev

libny: libny.a(ny.o) libny.a(alloc.o) libny.a(tcp.o) libny.a(error.o)

.c.a:
	@set -e; \
	echo '  CC $<'; \
	trap 'rm -f $*.o' EXIT; \
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< $(LDFLAGS); \
	$(AR) -r -c $@ $*.o

.SUFFIXES: .a .c
