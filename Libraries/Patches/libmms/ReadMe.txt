
Fixed by Carlo Bramini

https://bugs.launchpad.net/libmms/+bug/339946








See https://code.launchpad.net/~libmms-devel/libmms/win32 for more details.



Had to make 1 fix to get it to compile in Linux:

src/mms.c, line 184, added #ifdef to check for OS/compiler:

#ifdef G_OS_WIN32
#if defined (_MSC_VER) || defined (__MINGW) || defined(__MINGW32__)
/* required for MSVC 6.0 */
static gdouble
guint64_to_gdouble (guint64 value)
{
  if (value & G_GINT64_CONSTANT (0x8000000000000000))
    return (gdouble) ((gint64) value) + (gdouble) 18446744073709551616.;
  else
    return (gdouble) ((gint64) value);
}
#endif
#endif