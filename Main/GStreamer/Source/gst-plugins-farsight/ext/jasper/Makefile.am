plugin_LTLIBRARIES = libgstjasper.la

libgstjasper_la_SOURCES = gstjasper.c gstjasperenc.c

libgstjasper_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS)
libgstjasper_la_LIBADD = $(GST_LIBS) $(JASPER_LIBS)
libgstjasper_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS)

noinst_HEADERS = gstjasperenc.h
