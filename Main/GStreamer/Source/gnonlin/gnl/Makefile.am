built_header_configure = gnlversion.h

plugin_LTLIBRARIES = libgnl.la

libgnl_la_SOURCES =		\
	gnl.c			\
	gnlobject.c		\
	gnlcomposition.c	\
	gnloperation.c		\
	gnlsource.c		\
	gnlfilesource.c
libgnl_la_CFLAGS = $(GST_CFLAGS)
libgnl_la_LIBADD = $(GST_LIBS)
libgnl_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

gnl_headers =			\
	gnl.h			\
	gnlobject.h		\
	gnlcomposition.h	\
	gnltypes.h		\
	gnloperation.h		\
	gnlsource.h		\
	gnltypes.h		\
	gnlfilesource.h

#files built on make all/check/instal
BUILT_SOURCES =			\
	$(built_header_configure)

DISTCLEANFILE = $(CLEANFILES) $(built_header_configure)

noinst_HEADERS = $(gnl_headers) $(built_header_configure)

