
For a correct build order (although by no means the only one), take a look at the build scripts.

If you manually attempt to load these libraries, this is a correct order:


Windows (ls --format=single-column *.dll):

iconv.dll
pthreadGC2.dll
zlib1.dll
libbz2.dll
libxml2-2.dll
libglib-2.0-0.dll
libgobject-2.0-0.dll
libgthread-2.0-0.dll
libgmodule-2.0-0.dll
libgio-2.0-0.dll
liboil-0.3-0.dll
libjpeg.dll
libpng12-0.dll
libopenjpeg-2.dll
libgpg-error-0.dll
libgcrypt-11.dll
libgnutls-26.dll
libgnutls-extra-26.dll
libgnutlsxx-26.dll
libsoup-2.4-1.dll
libneon-27.dll
libfontconfig-1.dll
libfreetype-6.dll
libpango-1.0-0.dll
libpangocairo-1.0-0.dll
libpangoft2-1.0-0.dll
libpangowin32-1.0-0.dll
libpixman-1-0.dll
libcairo-2.dll
libschroedinger-1.0-0.dll
libmp3lame-0.dll
libspeex-1.dll
libspeexdsp-1.dll
libogg-0.dll
libtheora-0.dll
libtheoradec-1.dll
libtheoraenc-1.dll
libvorbis-0.dll
libvorbisenc-2.dll
libvorbisfile-3.dll
libx264-67.dll
libmms-0.dll
(FFmpeg is absent since it must be statically linked)


Linux:

libz.so
libxml2.so
libjpeg.so
libpng12.so
libglib-2.0.so
libgobject-2.0.so
libgmodule-2.0.so
libgthread-2.0.so
libgio-2.0.so
libcrypto.so
libssl.so
libneon.so
libfreetype.so
libfontconfig.so
libpixman-1.so
libcairo.so
libpango-1.0.so
libpangocairo-1.0.so
libpangoft2-1.0.so
libogg.so
libvorbis.so
libvorbisenc.so
libvorbisfile.so
libtheora.so
libtheoradec.so
libtheoraenc.so
libmms.so
libx264.so
