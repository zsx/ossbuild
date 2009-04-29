plugin_LTLIBRARIES = libgstjpeg2k.la

libgstjpeg2k_la_SOURCES = gstj2k.c gstj2kdec.c gstj2kenc.c gstyahooparse.c gstyahooenc.c

libgstjpeg2k_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS) -I$(LIBJ2K_PREFIX)/include
libgstjpeg2k_la_LIBADD = $(GST_LIBS)  
libgstjpeg2k_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -L$(LIBJ2K_PREFIX)/lib -lj2k

noinst_HEADERS = gstj2kdec.h gstj2kenc.h gstyahooparse.h gstyahooenc.h
