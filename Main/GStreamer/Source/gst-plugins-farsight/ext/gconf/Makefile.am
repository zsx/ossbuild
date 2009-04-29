plugin_LTLIBRARIES = libgstgconfelements2.la

libgstgconfelements2_la_SOURCES = gstgconf.c gstgconfv4l2src.c 

libgstgconfelements2_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(GCONF_CFLAGS)
libgstgconfelements2_la_LIBADD = $(GST_BASE_LIBS) -lgstinterfaces-$(GST_MAJORMINOR) \
								 $(GST_PLUGINS_BASE_LIBS) $(GCONF_LIBS)
libgstgconfelements2_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS = gstgconfv4l2src.h
