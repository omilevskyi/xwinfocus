PROG=		xwinfocus
SRCS=		main.c
MAN=

LOCALBASE?=	/usr/local

CFLAGS:=	-O3 -ffast-math -flto=full ${CFLAGS:N-O*}
CFLAGS+=	-I${LOCALBASE}/include
CFLAGS+=	-g
LDFLAGS+=	-Wl,-O3
LDFLAGS+=	-Wl,--as-needed
LDFLAGS+=	-Wl,--sort-common
LDFLAGS+=	-L${LOCALBASE}/lib
LDFLAGS+=	-lX11
LDFLAGS+=	-lxcb
LDFLAGS+=	-lXau
LDFLAGS+=	-lXdmcp

MK_PIE=		no
MK_RELRO=	no
MK_SSP=		no
MK_DEBUG_FILES=	no

NO_SHARED=	yes

NO_WCAST_ALIGN=	yes
WARNS?=		6

CLEANFILES+=	.depend .depend.main.o
CLEANDIRS+=	.ccls-cache

.MAIN: all

install-home: .PHONY
	${INSTALL} -s ${PROG} ${HOME}/bin

install: .PHONY
	${INSTALL} -s ${PROG} ${DESTDIR}${PREFIX}/bin

.sinclude "Makefile.local"
.include <bsd.prog.mk>
