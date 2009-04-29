plugin_LTLIBRARIES = libgstnetsim.la

libgstnetsim_la_SOURCES = gstnetsim.c

libgstnetsim_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS)
libgstnetsim_la_LIBADD = $(GST_LIBS_LIBS)
libgstnetsim_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -lgstrtp-@GST_MAJORMINOR@

noinst_HEADERS = gstnetsim.h
