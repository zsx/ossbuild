/* Trivial wrapper for win_iconv.c to make a GNU libiconv compatible iconv.dll
 *
 * This file is placed in the public domain.
 *
 * Tor Lillqvist <tml@iki.fi>, January 2008
 */

#include <errno.h>

#include "win_iconv.c"

__declspec(dllexport) 
int _libiconv_version = 0;

__declspec(dllexport) 
iconv_t
libiconv_open (const char* tocode, const char* fromcode)
{
  return iconv_open (tocode, fromcode);
}

__declspec(dllexport) 
int
libiconv_close (iconv_t cd)
{
  return iconv_close (cd);
}

__declspec(dllexport) 
size_t
libiconv (iconv_t cd, const char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft)
{
  return iconv (cd, inbuf, inbytesleft, outbuf, outbytesleft);
}

__declspec(dllexport) 
void
libiconv_set_relocation_prefix (const char *orig_prefix, const char *curr_prefix)
{
}

__declspec(dllexport) 
int
libiconvctl (iconv_t cd, int request, void* argument)
{
  errno = EINVAL;
  return -1;
}

__declspec(dllexport) 
void
libiconvlist (int (*do_one) (unsigned int namescount, const char * const * names, void* data), void *data)
{
  /* If somebody actually needs this to work, please mail me. */
  errno = EINVAL;
}

__declspec(dllexport) 
const char *
locale_charset (void)
{
  static char buf[20];

  /* Ignore the hardcoded aliases in GNU libiconv, just use the code page number.
   * If this causes trouble, please mail me.
   */
  sprintf (buf, "CP%u", GetACP ());

  return buf;
}
