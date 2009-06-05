plugin_LTLIBRARIES = libgstflump3dec.la

SOURCE_FILES = mp3-c.c mp3-c-synth.c
if USE_IPP
SOURCE_FILES += mp3-ipp.c
endif

libgstflump3dec_la_SOURCES = \
    flump3dec.c    \
    bitstream.c  \
    common.c      \
    decode.c      \
    mp3tl.c       \
    $(SOURCE_FILES)

noinst_HEADERS = \
    bitstream.h         \
    common.h  		\
    decode.h		\
    flump3dec.h		\
    mp3tl.h 		\
    mp3tl-priv.h        \
    table-bitalloc.h	\
    table-dewindow.h	\
    table-dewindow-ipp.h \
    table-huffdec.h     \
    mp3-cos-tables.h    \
    mp3-c.h		\
    mp3-ipp.h		\
    table-powtable.h \
    table-powtable-2.h

libgstflump3dec_la_LIBADD = $(GST_LIBS) $(IPP_LIBS) $(LIBOIL_LIBS) -lm
libgstflump3dec_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(CPU_TUNE_LDFLAGS)
libgstflump3dec_la_CFLAGS = \
	$(GST_ERROR) $(GST_CFLAGS) $(CPU_TUNE_CFLAGS) \
	$(IPP_INCLUDES) $(LIBOIL_CFLAGS)
