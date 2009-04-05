This is a repackaging of win_iconv by Yukihiro Nakadaira. The actual
source code in win_iconv.c is by him. I just added some small bits:
the trivial iconv.h header, the win_iconv_dll.c wrapper that makes it
possible to build win_iconv.c into a DLL that has the same API and ABI
as GNU libiconv, the src/Makefile.

--Tor Lillqvist <tml@iki.fi>, January 2008

