CPPFLAGS ?= -D_FORTIFY_SOURCE=2 -pedantic -Wall -Wstrict-overflow=5 \
-Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn
CFLAGS   ?= -pipe -O2 -fstack-protector
PREFIX   ?= usr/local

CPPFLAGS += -D_XOPEN_SOURCE=700 -D_BSD_SOURCE -std=c11 -pthread

libny    := ny.o error.o alloc.o tcp.o
testny   := test.c ./libny.so

libny: libny.so libny.a

libny.so: $(libny)
	@echo '  LD $@'; \
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(libny) $(LDFLAGS) -shared -lev

libny.a: $(libny)
	@echo '  AR $@'; \
	$(AR) -r -u -c $@ $(libny)

testny: $(testny)
	@echo '  CC $<'; \
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(testny) $(LDFLAGS) -lcheck

.c.o:
	@echo '  CC $<'; \
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $<

check: testny
	@./testny

.SUFFIXES: .c .o
.PHONY: check
