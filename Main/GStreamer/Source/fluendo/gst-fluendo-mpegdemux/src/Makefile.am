# plugindir is set in configure

plugin_LTLIBRARIES = libgstflumpegdemux.la

# for the next set of variables, rename the prefix if you renamed the .la

# sources used to compile this plugin
libgstflumpegdemux_la_SOURCES = \
	flumpegdemux.c	\
	gstmpegdesc.c	\
	gstmpegdemux.c	\
	gstpesfilter.c	\
	gstmpegtsdemux.c \
	flutspatinfo.c \
	flutspmtinfo.c \
	flutspmtstreaminfo.c \
	gstsectionfilter.c

# flags used to compile this plugin
# we use the GST_LIBS flags because we might be using plug-in libs
libgstflumpegdemux_la_CFLAGS = $(GST_CFLAGS) $(LIBOIL_CFLAGS)
libgstflumpegdemux_la_LIBADD = $(GST_LIBS) $(GST_BASE_LIBS) $(LIBOIL_LIBS)
libgstflumpegdemux_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) 

# headers we need but don't want installed
noinst_HEADERS =	\
	gstmpegdefs.h 	\
	gstmpegdesc.h 	\
	gstmpegdemux.h 	\
	gstpesfilter.h	\
	gstmpegtsdemux.h \
	flutspatinfo.h \
	flutspmtinfo.h \
	flutspmtstreaminfo.h \
	gstsectionfilter.h

