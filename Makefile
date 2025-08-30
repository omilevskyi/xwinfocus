PROG=		xwinfocus
SRCS=		${PROG}.c ${PROG}.h
MAN=		${PROG}.1

LOCALBASE?=	/usr/local

CFLAGS:=	-O3 -flto=full -funified-lto ${CFLAGS:N-O*}
CFLAGS+=	-DPROG='"${PROG}"'
CFLAGS+=	-I${LOCALBASE}/include

LDFLAGS+=	-Wl,-O3
LDFLAGS+=	-Wl,--lto=full
LDFLAGS+=	-Wl,--as-needed
LDFLAGS+=	-Wl,--sort-common
LDFLAGS+=	-L${LOCALBASE}/lib
LDFLAGS+=	-lX11
LDFLAGS+=	-lxcb
LDFLAGS+=	-lXau
LDFLAGS+=	-lXdmcp

MK_PIE?=	no
MK_RELRO?=	no
MK_SSP?=	no

NO_SHARED=	yes

.if empty(DEBUG)
MK_DEBUG_FILES=	no
.else
CFLAGS+=	-g
.endif

.if !empty(NO_LIST_WINDOWS)
CFLAGS+=	-DNO_LIST_WINDOWS
.endif

NO_WCAST_ALIGN=	yes
WARNS?=		6

CLEANFILES+=	.depend .depend.${PROG}.o ${PROG}.proj.cppcheck
CLEANDIRS+=	.ccls-cache ${PROG}-cppcheck-build-dir

.MAIN: all

install: .PHONY
	${INSTALL} -s ${PROG} ${DESTDIR}${PREFIX}/bin

.sinclude "Makefile.local"
.include <bsd.prog.mk>
