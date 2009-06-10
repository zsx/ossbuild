plugin_LTLIBRARIES = libgnl.la

libgnl_la_SOURCES =		\
	gnl.c			\
	gnlobject.c		\
	gnlcomposition.c	\
	gnlghostpad.c		\
	gnloperation.c		\
	gnlsource.c		\
	gnlfilesource.c
libgnl_la_CFLAGS = $(GST_CFLAGS)
libgnl_la_LIBADD = $(GST_LIBS)
libgnl_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgnl_la_LIBTOOLFLAGS = --tag=disable-static

gnl_headers =			\
	gnl.h			\
	gnlobject.h		\
	gnlcomposition.h	\
	gnltypes.h		\
	gnlghostpad.h		\
	gnloperation.h		\
	gnlsource.h		\
	gnltypes.h		\
	gnlfilesource.h

DISTCLEANFILE = $(CLEANFILES)

noinst_HEADERS = $(gnl_headers)

