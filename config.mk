# bgs version
VERSION = 0.7.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# Xinerama, comment if you don't want it
XINERAMALIBS = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# includes and libs
INCS = `pkg-config --cflags imlib2`
LIBS = `pkg-config --libs imlib2` ${XINERAMALIBS}

# flags
CPPFLAGS += -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS += -std=c99 -pedantic -Wall ${INCS} ${CPPFLAGS}
LDFLAGS += ${LIBS}

# compiler and linker
CC ?= cc
