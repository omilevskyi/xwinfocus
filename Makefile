PROG=		xwinfocus
SRCS=		${PROG}.c include/${PROG}.h fringe.c include/fringe.h option_string.c include/option_string.h
MAN=		man/${PROG}.1

LOCALBASE?=	/usr/local

CFLAGS:=	-O3 ${CFLAGS:N-O*}
LDFLAGS=	-Wl,-O3

CFLAGS+=	-fomit-frame-pointer -fno-unwind-tables -fno-asynchronous-unwind-tables

CFLAGS+=	-flto=full -funified-lto -fsanitize=cfi-icall
LDFLAGS+=	-Wl,--lto=full
LDFLAGS+=	-Wl,--lto-O3

CFLAGS+=	-faddrsig
LDFLAGS+=	-Wl,--icf=all

CFLAGS+=	-ffunction-sections
CFLAGS+=	-fdata-sections
LDFLAGS+=	-Wl,--gc-sections

CFLAGS+=	-fstack-protector-strong
CFLAGS+=	-ftrivial-auto-var-init=zero
CFLAGS+=	-fzero-call-used-regs=all
CFLAGS+=	-Wformat -Wformat-security -Werror=format-security

CFLAGS+=	-DPROG='"${PROG}"'
CFLAGS+=	-I${LOCALBASE}/include

LDFLAGS+=	-Wl,--build-id=none
LDFLAGS+=	-Wl,--as-needed
LDFLAGS+=	-Wl,--sort-common

LDFLAGS+=	-L${LOCALBASE}/lib
LDFLAGS+=	-lX11
LDFLAGS+=	-lxcb
LDFLAGS+=	-lXau
LDFLAGS+=	-lXdmcp

TEST_SRCS=	test/test_fringe.c test/test_option_string.c

TEST_CFLAGS=	${CFLAGS:N-Wmissing-variable-declarations} -I.
TEST_LDFLAGS=	-L${LOCALBASE}/lib -lcriterion

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

CLEANFILES+=	.depend .depend.*.o ${PROG}.proj.cppcheck ${TEST_SRCS:R} ${TEST_SRCS:R:S/$/.o/}
CLEANDIRS+=	.ccls-cache ${PROG}-cppcheck-build-dir

.MAIN: all

install: .PHONY
	${INSTALL} -s ${PROG} ${DESTDIR}${PREFIX}/bin

test: .PHONY
.for _i in ${TEST_SRCS}
	${CC} ${TEST_CFLAGS} ${TEST_LDFLAGS} -o ${_i:R} ${_i} ${_i:C/.*test_//}
	./${_i:R}
.endfor

.sinclude "Makefile.local"
.include <bsd.prog.mk>
