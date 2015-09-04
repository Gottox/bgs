# bgs version
VERSION = 0.8

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# Xinerama, comment if you don't want it
XINERAMALIBS = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# includes and libs
INCS = `pkg-config --cflags imlib2 x11`
LIBS = `pkg-config --libs imlib2 x11` ${XINERAMALIBS}

# flags
CPPFLAGS += -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS += -std=c99 -pedantic -Wall ${INCS} ${CPPFLAGS}
LDFLAGS += -lm ${LIBS}

# compiler and linker
CC ?= cc
