plugin_LTLIBRARIES = libgstrtpjitterbuffer.la

glib_enum_define = GST_RTP_JITTER_BUFFER
glib_enum_prefix = gstrtpjitterbuffer

include $(top_srcdir)/common/glib-gen.mak

BUILT_SOURCES = gstrtpjitterbuffer-marshal.c \
		gstrtpjitterbuffer-marshal.h

CLEANFILES = $(BUILT_SOURCES)

libgstrtpjitterbuffer_la_SOURCES = gstrtpjitterbuffer.c async_jitter_queue.c

nodist_libgstrtpjitterbuffer_la_SOURCES = gstrtpjitterbuffer-marshal.c gstrtpjitterbuffer-marshal.h

libgstrtpjitterbuffer_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS)
libgstrtpjitterbuffer_la_LIBADD = $(GST_LIBS_LIBS)
libgstrtpjitterbuffer_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -lgstrtp-@GST_MAJORMINOR@

noinst_HEADERS = gstrtpjitterbuffer.h async_jitter_queue.h
nodist_noinst_HEADERS = gstrtpjitterbuffer-marshal.h

EXTRA_DIST = gstrtpjitterbuffer-marshal.list
