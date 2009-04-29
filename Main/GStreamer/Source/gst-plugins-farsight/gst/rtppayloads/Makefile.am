plugin_LTLIBRARIES = libgstrtppayloads.la

libgstrtppayloads_la_SOURCES = \
    gstmsgsmpayload.c \
    gstmsgsmpayload.h \
    gstmsgsmdepayload.c \
    gstmsgsmdepayload.h \
    gstrtpcnpayload.c \
    gstrtpcnpayload.h \
    gstrtpcndepayload.c \
    gstrtpcndepayload.h \
    gstrtppayloads.c

libgstrtppayloads_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(ERROR_CFLAGS) -DEXTERN_BUF -DRTP_SUPPORT
libgstrtppayloads_la_LIBADD = $(GST_LIBS_LIBS)
libgstrtppayloads_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) -lgstrtp-@GST_MAJORMINOR@
