plugin_LTLIBRARIES = libgstmsgsm.la

if USE_GSM_WAV49
  libgstmsgsm_la_SOURCES = gstgsm.c gstmsgsmenc.c gstmsgsmdec.c
else
  libgstmsgsm_la_SOURCES = gstgsm.c
endif

libgstmsgsm_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(ERROR_CFLAGS)
libgstmsgsm_la_LIBADD = $(GSM_LIBS)
libgstmsgsm_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LIBS)

noinst_HEADERS = gstmsgsmenc.h gstmsgsmdec.h
