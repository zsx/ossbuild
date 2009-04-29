plugin_LTLIBRARIES = libgstrtpdemux.la

noinst_PROGRAMS = test_rtpdemux

test_rtpdemux_SOURCES = test_rtpdemux.c
test_rtpdemux_CFLAGS = $(GST_CFLAGS)
test_rtpdemux_LDADD = $(GST_LIBS)

libgstrtpdemux_la_SOURCES = gstrtpdemux.c gstrtpdemux-marshal.c

libgstrtpdemux_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS) -DEXTERN_BUF -DRTP_SUPPORT
libgstrtpdemux_la_LIBADD = $(GST_LIBS_LIBS)
libgstrtpdemux_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -lgstrtp-@GST_MAJORMINOR@

#noinst_HEADERS = gstrrtpdemux.h

glib_enum_define = GST_RTP_DEMUX
glib_enum_prefix = gst_rtp_demux

include $(top_srcdir)/common/glib-gen.mak

BUILT_SOURCES = gstrtpdemux-marshal.c \
		gstrtpdemux-marshal.h

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = gstrtpdemux-marshal.list
