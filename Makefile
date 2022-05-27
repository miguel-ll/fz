CFLAGS+=-Wall -Wextra -std=c99 -O2
PREFIX?=/usr/local

INSTALL=install
INSTALL_PROGRAM=$(INSTALL)

fz: fz.o match.o tty.o
	$(CC) $(CCFLAGS) -o $@ $^

%.o: %.c fz.h
	$(CC) $(CFLAGS) -c -o $@ $<

install: fz
	$(INSTALL_PROGRAM) fz $(DESTDIR)$(PREFIX)/bin/fz
	$(RM) fz *.o

uninstall:
	$(RM) /usr/local/bin/fz

clean:
	$(RM) fz *.o

.PHONY: test all clean install
