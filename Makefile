VERSION = 0

CC = cc
CSTANDARD = -std=c99
CWARNINGS = -Wall -Wextra -Wshadow -pedantic
ARCH = native
COPTIMIZE = -O2 -march=$(ARCH) -flto
CFLAGS = $(CSTANDARD) $(CWARNINGS) $(COPTIMIZE)
LDFLAGS = $(CFLAGS)

DVERSION = -DVERSION=$(VERSION)

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

all: sloc

sloc: sloc.o
	$(CC) $(LDFLAGS) $^ -o $@

sloc.o: sloc.c
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -f sloc $(DESTDIR)$(BINDIR)/sloc
	chmod 755 $(DESTDIR)$(BINDIR)/sloc

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/sloc

clean:
	rm -rf $(LOCAL_OBJDIR) $(LOCAL_DEPDIR)

options:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"

.PHONY: all clean install uninstall options
