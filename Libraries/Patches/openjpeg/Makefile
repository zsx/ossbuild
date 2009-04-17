# MinGW (i.e. GNU toolchain targeting Win32) makefile for OpenJPEG

VER_MAJOR = 2
VER_MINOR = 1.3.0

SRCS = \
	./libopenjpeg/bio.c ./libopenjpeg/cio.c ./libopenjpeg/dwt.c \
	./libopenjpeg/event.c ./libopenjpeg/image.c ./libopenjpeg/j2k.c \
	./libopenjpeg/j2k_lib.c ./libopenjpeg/jp2.c ./libopenjpeg/jpt.c \
	./libopenjpeg/mct.c ./libopenjpeg/mqc.c ./libopenjpeg/openjpeg.c \
	./libopenjpeg/pi.c ./libopenjpeg/raw.c ./libopenjpeg/t1.c \
	./libopenjpeg/t2.c ./libopenjpeg/tcd.c ./libopenjpeg/tgt.c
INCLS = \
	./libopenjpeg/bio.h ./libopenjpeg/cio.h ./libopenjpeg/dwt.h \
	./libopenjpeg/event.h ./libopenjpeg/fix.h ./libopenjpeg/image.h \
	./libopenjpeg/int.h ./libopenjpeg/j2k.h ./libopenjpeg/j2k_lib.h \
	./libopenjpeg/jp2.h ./libopenjpeg/jpt.h ./libopenjpeg/mct.h \
	./libopenjpeg/mqc.h ./libopenjpeg/openjpeg.h ./libopenjpeg/pi.h \
	./libopenjpeg/raw.h ./libopenjpeg/t1.h ./libopenjpeg/t2.h \
	./libopenjpeg/tcd.h ./libopenjpeg/tgt.h ./libopenjpeg/opj_malloc.h \
	./libopenjpeg/opj_includes.h
INCLUDE = -Ilibopenjpeg

# General configuration variables:
CC = gcc
AR = ar

PREFIX = /dummy
INSTALL_BINDIR = $(PREFIX)/bin
INSTALL_LIBDIR = $(PREFIX)/lib
INSTALL_INCLUDE = $(PREFIX)/include

COMPILERFLAGS = -Wall -O3 -ffast-math -std=c99 -DWIN32 -DOPJ_EXPORTS

MODULES = $(SRCS:.c=.o)
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)

TARGET  = openjpeg
SHAREDLIB = lib$(TARGET)-$(VER_MAJOR).dll
IMPLIB = lib$(TARGET).dll.a


default: all

all: OpenJPEG

dist: OpenJPEG
	install -d dist
	install $(SHAREDLIB) dist
	install $(IMPLIB) dist
	install libopenjpeg/openjpeg.h dist

OpenJPEG: $(SHAREDLIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(SHAREDLIB): $(MODULES)
	$(CC) -shared -o $@ -Wl,--kill-at -Wl,--out-implib,$(IMPLIB) $(MODULES) $(LIBRARIES)

install: OpenJPEG
	install -d '$(DESTDIR)$(INSTALL_LIBDIR)' '$(DESTDIR)$(INSTALL_INCLUDE)'
	install $(SHAREDLIB) '$(DESTDIR)$(INSTALL_BINDIR)'
	install $(IMPLIB) '$(DESTDIR)$(INSTALL_LIBDIR)'
	install ./libopenjpeg/openjpeg.h '$(DESTDIR)$(INSTALL_INCLUDE)'

clean:
	rm -rf dist u2dtmp* $(MODULES) $(SHAREDLIB) $(IMPLIB)