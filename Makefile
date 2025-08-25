PROG=	xwinfocus
SRCS=	main.c
MAN=

CFLAGS:=	-O3 -ffast-math ${CFLAGS:N-O*} -flto=full
CFLAGS+=	-I/usr/local/include
LDFLAGS+=	-Wl,-O3
LDFLAGS+=	-Wl,--as-needed
LDFLAGS+=	-Wl,--sort-common
LDFLAGS+=	-L/usr/local/lib
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
	${INSTALL} -s ${.CURDIR}/${PROG} ${HOME}/bin/${PROG}

.include <bsd.prog.mk>
