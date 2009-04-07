# Linux makefile for OpenJPEG

VER_MAJOR = 1
VER_MINOR = 0.0

SRCS = ./libopenjpeg/bio.c ./libopenjpeg/cio.c ./libopenjpeg/dwt.c ./libopenjpeg/event.c ./libopenjpeg/image.c ./libopenjpeg/j2k.c ./libopenjpeg/j2k_lib.c ./libopenjpeg/jp2.c ./libopenjpeg/jpt.c ./libopenjpeg/mct.c ./libopenjpeg/mqc.c ./libopenjpeg/openjpeg.c ./libopenjpeg/pi.c ./libopenjpeg/raw.c ./libopenjpeg/t1.c ./libopenjpeg/t2.c ./libopenjpeg/tcd.c ./libopenjpeg/tgt.c
INCLS = ./libopenjpeg/bio.h ./libopenjpeg/cio.h ./libopenjpeg/dwt.h ./libopenjpeg/event.h ./libopenjpeg/fix.h ./libopenjpeg/image.h ./libopenjpeg/int.h ./libopenjpeg/j2k.h ./libopenjpeg/j2k_lib.h ./libopenjpeg/jp2.h ./libopenjpeg/jpt.h ./libopenjpeg/mct.h ./libopenjpeg/mqc.h ./libopenjpeg/openjpeg.h ./libopenjpeg/pi.h ./libopenjpeg/raw.h ./libopenjpeg/t1.h ./libopenjpeg/t2.h ./libopenjpeg/tcd.h ./libopenjpeg/tgt.h ./libopenjpeg/opj_includes.h
INCLUDE = -Ilibopenjpeg

# General configuration variables:
CC = gcc
AR = ar

INSTALLDIR = /usr/lib

# Converts cr/lf to just lf
DOS2UNIX = dos2unix

COMPILERFLAGS = -O3
LIBRARIES = -lstdc++

MODULES = $(SRCS:.c=.o)
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)

TARGET  = openjpeg
STATICLIB = lib$(TARGET).a
SHAREDLIB = lib$(TARGET)-$(VER_MAJOR).$(VER_MINOR).so
LIBNAME = lib$(TARGET).so.$(VER_MAJOR)



default: all

all: dist

dist: OpenJPEG
	mkdir dist
	cp *.a dist/
	cp *.so dist/
	cp libopenjpeg/openjpeg.h dist/

dos2unix:
	@$(DOS2UNIX) $(SRCS) $(INCLS)

OpenJPEG: $(STATICLIB) $(SHAREDLIB)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(STATICLIB): $(MODULES)
	$(AR) r $@ $(MODULES)

$(SHAREDLIB): $(MODULES)
	$(CC) -s -shared -Wl,-soname,$(LIBNAME) -o $@ $(MODULES) $(LIBRARIES)

install:
	install -m 644 -o root -g root $(STATICLIB) $(INSTALLDIR)
	install -m 755 -o root -g root $(SHAREDLIB) $(INSTALLDIR)
	ln -sf $(SHAREDLIB) $(INSTALLDIR)/$(LIBNAME)
	ldconfig

clean:
	rm -rf core dist/ u2dtmp* $(MODULES) $(STATICLIB) $(SHAREDLIB) $(LIBNAME)

osx:
	make -f Makefile.osx

osxinstall:
	make -f Makefile.osx install

osxclean:
	make -f Makefile.osx clean
