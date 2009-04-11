#ifndef __MSVCRT_COMPAT_H__
#define __MSVCRT_COMPAT_H__

/* Because of annoying CRTs */
#ifdef _MSC_VER
#  define open _open
#  define close _close
#  define read _read
#  define write _write
#  define lseek _lseek
#  define fdopen _fdopen
#  define dup _dup
#  define strupr _strupr
#endif

#endif /* __MSVCRT_COMPAT_H__ */