
/* Created to override some of the pedantic compiler options specified in msvc-pragmas.h
 * These need to be turned off for libx264 to compile correctly.
 */

#ifndef _MSC_VER
#pragma error "This header is for Microsoft VC only."
#endif /* _MSC_VER */

/* Make MSVC more pedantic, this is a recommended pragma list
 * from _Win32_Programming_ by Rector and Newcomer.
 */
#pragma warning(disable:4002) /* too many actual parameters for macro */
#pragma warning(disable:4003) /* not enough actual parameters for macro */
#pragma warning(disable:4013) /* 'function' undefined; assuming extern returning int */
#pragma warning(disable:4020) /* too many actual parameters */
#pragma warning(disable:4021) /* too few actual parameters */

/* Define a C99 function that x264 uses.
 */
#define log2f(x) ((float)( log(x) / log(2) ))
