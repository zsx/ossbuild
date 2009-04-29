plugin_LTLIBRARIES = libgstjrtp.la

if USE_JRTP
libgstjrtp_la_SOURCES = jrtplib_c.cpp gstjrtp.c gstrtpsend.c gstrtprecv.c gstrtpbin.c
else
libgstjrtp_la_SOURCES = gstjrtp.c gstrtpsend.c gstrtprecv.c gstrtpbin.c
endif

if USE_JRTP
libgstjrtp_la_CFLAGS = $(GST_CFLAGS) $(JRTP_CFLAGS) $(JTHREAD_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS)
libgstjrtp_la_LIBADD = $(JRTP_LIBS) $(JTHREAD_LIBS) $(GST_PLUGINS_BASE_LIBS) -lstdc++ -lgstnet-@GST_MAJORMINOR@ -lgstnetbuffer-@GST_MAJORMINOR@ -lgstrtp-@GST_MAJORMINOR@
libgstjrtp_la_CXXFLAGS = $(GST_CFLAGS) $(JRTP_CFLAGS) $(JTHREAD_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS)
libgstjrtp_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
else
libgstjrtp_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS)
libgstjrtp_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) -lgstnet-@GST_MAJORMINOR@ -lgstnetbuffer-@GST_MAJORMINOR@ -lgstrtp-@GST_MAJORMINOR@
libgstjrtp_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
endif

noinst_HEADERS = jrtplib_c.h gstrtpsend.h gstrtprecv.h gstrtpbin.h default_pt.h
