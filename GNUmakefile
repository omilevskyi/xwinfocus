PROG:=		xwinfocus
SRCS:=		$(PROG).c fringe.c option_string.c
HDRS:=		include/$(PROG).h include/fringe.h include/option_string.h
MANPAGE:=	man/$(PROG).1
OBJS:=		$(SRCS:.c=.o)

CFLAGS= 	-O3 -pipe -flto -ffunction-sections -fdata-sections -fno-semantic-interposition
CFLAGS+=	-Wall -Wextra -Wformat -Wformat-security -Werror=format-security -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings

CFLAGS+=	-DPROG=\"$(PROG)\"

ifneq ($(strip $(VERSION)),)
CFLAGS+=	-DVERSION=\"$(VERSION)\"
endif

ifneq ($(strip $(COMMIT_HASH)),)
CFLAGS+=	-DCOMMIT_HASH=\"$(COMMIT_HASH)\"
endif

LDFLAGS:=	-O3 -flto
LDFLAGS+=	-Wl,-O3
LDFLAGS+=	-Wl,-flto
LDFLAGS+=	-Wl,--gc-sections
LDFLAGS+=	-Wl,--as-needed
LDFLAGS+=	-Wl,--sort-common
LDFLAGS+=	-Wl,-z,pack-relative-relocs
LDFLAGS+=	-Wl,-z,defs
LDFLAGS+=	-lX11
LDFLAGS+=	-lxcb
LDFLAGS+=	-lXau
LDFLAGS+=	-lXdmcp

TEST_CFLAGS:=	$(CFLAGS) -I.
TEST_LDFLAGS:=	-lcriterion

INSTALL:=	install
INSTALL_BIN:=	$(INSTALL) -s -m 0755
INSTALL_MAN:=	$(INSTALL) -m 0644

.PHONY: all clean test install

all: $(PROG) $(MANPAGE).gz

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

$(MANPAGE).gz: $(MANPAGE)
	gzip --to-stdout --no-name $(MANPAGE) > $(notdir $(MANPAGE)).gz

test:
	find test -type f -name '*.c' -print | while read -r f; do \
		bin_file=$$(basename --suffix=.c $${f}); \
		c_source=$${f#test/test_}; \
		$(CC) $(TEST_CFLAGS) $(TEST_LDFLAGS) -o $${bin_file} $${f} $${c_source} && ./$${bin_file}; \
	done

install:
	$(INSTALL_BIN) $(PROG) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL_MAN) $(notdir $(MANPAGE)).gz $(DESTDIR)$(PREFIX)/share/man/man1

clean:
	rm -f -- $(PROG) *.o *.gz
