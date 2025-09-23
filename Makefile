PROG=		xwinfocus
SRCS=		${PROG}.c include/${PROG}.h fringe.c include/fringe.h option_string.c include/option_string.h
MAN=		man/${PROG}.1

LLVM_DEFAULT?=	19
CLANG_FORMAT?=	clang-format${LLVM_DEFAULT}
SCAN_BUILD?=	scan-build${LLVM_DEFAULT}
CPPCHECK?=	cppcheck

LOCALBASE?=	/usr/local

.if empty(.TARGETS:Msanitize)
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
#CFLAGS+=	-fno-unique-section-names	# no effect

CFLAGS+=	-fstack-protector-strong
CFLAGS+=	-ftrivial-auto-var-init=zero
CFLAGS+=	-fzero-call-used-regs=all
CFLAGS+=	-Wformat -Wformat-security -Werror=format-security

LDFLAGS+=	-Wl,--build-id=none
LDFLAGS+=	-Wl,--as-needed
LDFLAGS+=	-Wl,--sort-common

MK_RELRO=	no
MK_PIE=		no
NO_SHARED=	yes
.else
DEBUG=		yes
CFLAGS=		-O1 -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS=	-Wl,-O1 -fsanitize=address,undefined
MK_RELRO=	yes
MK_PIE=		yes
.undef NO_SHARED
.endif

CFLAGS+=	-DPROG='"${PROG}"'
CFLAGS+=	-I${LOCALBASE}/include

LDFLAGS+=	-L${LOCALBASE}/lib
LDFLAGS+=	-lX11
LDFLAGS+=	-lxcb
LDFLAGS+=	-lXau
LDFLAGS+=	-lXdmcp

TEST_SRCS!=	find test -name '*.c' -type f

TEST_CFLAGS=	${CFLAGS:N-Wmissing-variable-declarations} -I.
TEST_LDFLAGS=	-L${LOCALBASE}/lib -lcriterion

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

CLEANFILES+=	.depend .depend.*.o ${PROG}.proj.cppcheck ${PROG}.debug ${PROG}.full ${TEST_SRCS:R} ${TEST_SRCS:R:S/$/.o/}
CLEANDIRS+=	.ccls-cache ${PROG}-cppcheck-build-dir

.MAIN: all

install: .PHONY
	${INSTALL} -s ${PROG} ${DESTDIR}${PREFIX}/bin

test: .PHONY
.for _i in ${TEST_SRCS}
	${CC} ${TEST_CFLAGS} ${TEST_LDFLAGS} -o ${_i:R} ${_i} ${_i:C/.*test_//}
	./${_i:R}
.endfor

fmt: .PHONY
	${CLANG_FORMAT} --verbose -i ${SRCS} ${TEST_SRCS}

analyze: .PHONY
	${SCAN_BUILD} --use-cc=clang --status-bugs make

cppcheck: .PHONY
	${CPPCHECK} --enable=all --inconclusive --check-level=exhaustive --force --error-exitcode=1 \
		--language=c --std=c99 --platform=unix64 \
		--suppress=missingIncludeSystem \
		--suppress=checkersReport ${SRCS:M*.c}

sanitize: .PHONY all
	./${PROG} --list

.sinclude "Makefile.local"
.include <bsd.prog.mk>
