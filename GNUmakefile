PROG:=		xwinfocus
SRCS:=		$(PROG).c fringe.c option_string.c
HDRS:=		include/$(PROG).h include/fringe.h include/option_string.h
MANPAGE:=	man/$(PROG).1
OBJS:=		$(SRCS:.c=.o)

CFLAGS=		-O3 -flto -DPROG=\"$(PROG)\"

ifneq ($(strip $(VERSION)),)
CFLAGS+=	-DVERSION=\"$(VERSION)\"
endif

ifneq ($(strip $(COMMIT_HASH)),)
CFLAGS+=	-DCOMMIT_HASH=\"$(COMMIT_HASH)\"
endif

LDFLAGS:=	-O3 -flto
LDFLAGS+=	-Wl,--as-needed
LDFLAGS+=	-Wl,--sort-common
LDFLAGS+=	-lX11
LDFLAGS+=	-lxcb
LDFLAGS+=	-lXau
LDFLAGS+=	-lXdmcp

INSTALL:=	install
INSTALL_BIN:=	$(INSTALL) -s -m 0755
INSTALL_MAN:=	$(INSTALL) -m 0644

.PHONY: all clean install

all: $(PROG) $(MANPAGE).gz

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

$(MANPAGE).gz: $(MANPAGE)
	gzip --to-stdout --no-name $(MANPAGE) > $(notdir $(MANPAGE)).gz

install:
	$(INSTALL_BIN) $(PROG) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL_MAN) $(notdir $(MANPAGE)).gz $(DESTDIR)$(PREFIX)/share/man/man1

clean:
	rm -f -- $(PROG) *.o *.gz
