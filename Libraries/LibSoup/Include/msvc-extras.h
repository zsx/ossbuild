
/* Created to fill in some missing functions needed to compile this under MSVC.
 */

#ifndef MSVC_EXTRAS_H
#define MSVC_EXTRAS_H

#ifndef _MSC_VER
#pragma error "This header is for Microsoft VC only."
#endif /* _MSC_VER */

#pragma once



/* Look in msvc-extras.c
 */
#include <stdio.h>
#include <stdarg.h>
int snprintf(char *buffer, size_t count, const char *fmt, ...);



/* Define MSVC-equivalents for UNIX types/functions
 */

#if defined(__INT_MAX__) && __INT_MAX__ == 2147483647
typedef int _ssize_t;
#else
typedef long _ssize_t;
#endif
typedef _ssize_t ssize_t;

#include <process.h>
#define _getpid getpid
typedef int pid_t;

//#define	_sntprintf	_snprintf
//#define _snprintf snprintf


//#define __restrict__ 
//#define __MINGW_NOTHROW

//#ifndef __VALIST
//#ifdef __GNUC__
//#define __VALIST __gnuc_va_list
//#else
//#define __VALIST char*
//#endif
//#endif /* defined __VALIST  */
//
//#ifndef _FILE_DEFINED
//#define	_FILE_DEFINED
//typedef struct _iobuf
//{
//	char*	_ptr;
//	int	_cnt;
//	char*	_base;
//	int	_flag;
//	int	_file;
//	int	_charbuf;
//	int	_bufsiz;
//	char*	_tmpfname;
//} FILE;
//#endif	/* Not _FILE_DEFINED */

//int __cdecl __MINGW_NOTHROW snprintf (char *, size_t, const char *, ...);
//int __cdecl __MINGW_NOTHROW vsnprintf (char *, size_t, const char *, __VALIST);
//
//int __cdecl __MINGW_NOTHROW vscanf (const char * __restrict__, __VALIST);
//int __cdecl __MINGW_NOTHROW vfscanf (FILE * __restrict__, const char * __restrict__,
//		     __VALIST);
//int __cdecl __MINGW_NOTHROW vsscanf (const char * __restrict__,
//		     const char * __restrict__, __VALIST);


#endif /* MSVC_EXTRAS_H */
