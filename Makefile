PROG=		xwinfocus
SRCS=		${PROG}.c include/${PROG}.h fringe.c include/fringe.h option_string.c include/option_string.h
MAN=		man/${PROG}.1

LOCALBASE?=	/usr/local

CFLAGS:=	-O3 ${CFLAGS:N-O*}
CFLAGS+=	-flto=full -funified-lto -fsanitize=cfi-icall
CFLAGS+=	-fomit-frame-pointer
CFLAGS+=	-faddrsig
CFLAGS+=	-ffunction-sections
CFLAGS+=	-fdata-sections
CFLAGS+=	-fstack-protector-strong
CFLAGS+=	-ftrivial-auto-var-init=zero
CFLAGS+=	-fzero-call-used-regs=all
CFLAGS+=	-Wformat -Wformat-security -Werror=format-security
CFLAGS+=	-DPROG='"${PROG}"'
CFLAGS+=	-I${LOCALBASE}/include

LDFLAGS=	-Wl,-O3
LDFLAGS+=	-Wl,--lto=full
LDFLAGS+=	-Wl,--icf=all
LDFLAGS+=	-Wl,--gc-sections
LDFLAGS+=	-Wl,--build-id=none
LDFLAGS+=	-Wl,--as-needed
LDFLAGS+=	-Wl,--sort-common
LDFLAGS+=	-L${LOCALBASE}/lib
LDFLAGS+=	-lX11
LDFLAGS+=	-lxcb
LDFLAGS+=	-lXau
LDFLAGS+=	-lXdmcp

TEST_SRCS=	test/test_fringe.c test/test_option_string.c

TEST_CFLAGS=	${CFLAGS:N-Wmissing-variable-declarations}
TEST_LDFLAGS=	${LDFLAGS:N-static}
TEST_LDFLAGS+=	-lcriterion

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
	${CC} ${TEST_CFLAGS} ${TEST_LDFLAGS} -o ${_i:R} ${_i} ${_i:S/^test_//}
	./${_i:R}
.endfor

.sinclude "Makefile.local"
.include <bsd.prog.mk>
