plugin_LTLIBRARIES = libgstice.la

libgstice_la_SOURCES = gstice.c gsticesink.c gsticesrc.c jingle_c.cpp

libgstice_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(JINGLEBASE_CFLAGS) $(JINGLEP2P_CFLAGS)
libgstice_la_LIBADD = $(JINGLEBASE_LIBS) $(JINGLEP2P_LIBS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -lrt -lpthread -lstdc++ -lgstnet-@GST_MAJORMINOR@ -lgstnetbuffer-@GST_MAJORMINOR@
libgstice_la_CXXFLAGS = $(GST_CFLAGS) $(JINGLEBASE_CFLAGS) $(JINGLEP2P_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS)
libgstice_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

noinst_HEADERS =  gsticesrc.h gsticesink.h jingle_c.h
