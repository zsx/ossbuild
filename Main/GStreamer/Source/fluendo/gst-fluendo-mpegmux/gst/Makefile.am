plugin_LTLIBRARIES = libgstflumpegmux.la

libgstflumpegmux_la_SOURCES = \
	flumpegtsmux.c \
	flumpegtsmux_h264.c \
	flumpegtsmux_aac.c

libgstflumpegmux_la_CFLAGS = $(TSMUX_CFLAGS) $(GST_CFLAGS)
libgstflumpegmux_la_LIBADD = $(TSMUX_LIBS) $(GST_LIBS) $(GST_BASE_LIBS)
libgstflumpegmux_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) -dynamic

noinst_HEADERS = \
	flumpegtsmux.h \
	flumpegtsmux_h264.h \
	flumpegtsmux_aac.h
