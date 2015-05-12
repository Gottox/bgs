# bgs version
VERSION = 0.7

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

IMLIB2INC = /usr/include/imlib2
IMLIB2LIB = /usr/lib/imlib2/

# Xinerama, comment if you don't want it
XINERAMALIBS = -L${X11LIB} -lXinerama
XINERAMAFLAGS = -DXINERAMA

# includes and libs
INCS = -I${X11INC} -I ${IMLIB2INC}
LIBS = -lm -L${X11LIB} -lX11 ${XINERAMALIBS} -L${IMLIB2LIB} -lImlib2 -lm

# flags
CFLAGS += -std=c99 -pedantic -Wall ${INCS} -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
LDFLAGS += ${LIBS}

# compiler and linker
CC ?= cc
