# MinGW (i.e. GNU toolchain targeting Win32) makefile for OpenJPEG

VER_MAJOR = 2
VER_MINOR = 1.3.0

SRCS = \
	./Source/libopenjpeg/bio.c ./Source/libopenjpeg/cio.c ./Source/libopenjpeg/dwt.c \
	./Source/libopenjpeg/event.c ./Source/libopenjpeg/image.c ./Source/libopenjpeg/j2k.c \
	./Source/libopenjpeg/j2k_lib.c ./Source/libopenjpeg/jp2.c ./Source/libopenjpeg/jpt.c \
	./Source/libopenjpeg/mct.c ./Source/libopenjpeg/mqc.c ./Source/libopenjpeg/openjpeg.c \
	./Source/libopenjpeg/pi.c ./Source/libopenjpeg/raw.c ./Source/libopenjpeg/t1.c \
	./Source/libopenjpeg/t2.c ./Source/libopenjpeg/tcd.c ./Source/libopenjpeg/tgt.c
INCLS = \
	./Source/libopenjpeg/bio.h ./Source/libopenjpeg/cio.h ./Source/libopenjpeg/dwt.h \
	./Source/libopenjpeg/event.h ./Source/libopenjpeg/fix.h ./Source/libopenjpeg/image.h \
	./Source/libopenjpeg/int.h ./Source/libopenjpeg/j2k.h ./Source/libopenjpeg/j2k_lib.h \
	./Source/libopenjpeg/jp2.h ./Source/libopenjpeg/jpt.h ./Source/libopenjpeg/mct.h \
	./Source/libopenjpeg/mqc.h ./Source/libopenjpeg/openjpeg.h ./Source/libopenjpeg/pi.h \
	./Source/libopenjpeg/raw.h ./Source/libopenjpeg/t1.h ./Source/libopenjpeg/t2.h \
	./Source/libopenjpeg/tcd.h ./Source/libopenjpeg/tgt.h ./Source/libopenjpeg/opj_malloc.h \
	./Source/libopenjpeg/opj_includes.h
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
	install ./Source/libopenjpeg/openjpeg.h '$(DESTDIR)$(INSTALL_INCLUDE)'

clean:
	rm -rf dist u2dtmp* $(MODULES) $(SHAREDLIB) $(IMPLIB)