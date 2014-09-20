# bgs - back ground setter
# See LICENSE file for copyright and license details.

include config.mk

SRC = bgs.c
OBJ = ${SRC:.c=.o}

all: options bgs

options:
	@echo bgs build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

bgs: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f bgs ${OBJ} bgs-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p bgs-${VERSION}
	@cp -R LICENSE Makefile README config.mk \
		bgs.1 ${SRC} bgs-${VERSION}
	@tar -cf bgs-${VERSION}.tar bgs-${VERSION}
	@gzip bgs-${VERSION}.tar
	@rm -rf bgs-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f bgs ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/bgs
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < bgs.1 > ${DESTDIR}${MANPREFIX}/man1/bgs.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/bgs.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/bgs
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/bgs.1

.PHONY: all options clean dist install uninstall
