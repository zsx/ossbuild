/* config.h.in.  Generated from configure.ac by autoheader.  */
/* This copy of config.h.in is specifically for win32 Visual Studio builds */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#define ENABLE_NLS 1

/* gettext package name */
#define GETTEXT_PACKAGE "farsight2"

/* Farsight2 license */
#define FS2_LICENSE "LGPL"

/* The path were plugins are installed and search by default */
/* !!!HACK!!! This will most likley never work, but default for now !!!HACK!!!*/
#define FS2_PLUGIN_PATH "c:\\farsight2-0.0.9\\"

/* macro to use to show function name */
#undef GST_FUNCTION

/* Defined if gcov is enabled to force a rebuild due to config.h changing */
#undef GST_GCOV_ENABLED

/* plugin install helper script */
#undef GST_INSTALL_PLUGINS_HELPER

/* Default errorlevel to use */
#define GST_LEVEL_DEFAULT GST_LEVEL_ERROR

/* GStreamer major.minor version */
#define GST_MAJORMINOR "0.10"

/* package name in plugins */
#undef GST_PACKAGE_NAME

/* package origin */
#undef GST_PACKAGE_ORIGIN

/* Define if the host CPU is an Alpha */
#undef HAVE_CPU_ALPHA

/* Define if the host CPU is an ARM */
#undef HAVE_CPU_ARM

/* Define if the host CPU is a CRIS */
#undef HAVE_CPU_CRIS

/* Define if the host CPU is a CRISv32 */
#undef HAVE_CPU_CRISV32

/* Define if the host CPU is a HPPA */
#undef HAVE_CPU_HPPA

/* Define if the host CPU is an x86 */
#undef HAVE_CPU_I386

/* Define if the host CPU is a IA64 */
#undef HAVE_CPU_IA64

/* Define if the host CPU is a M68K */
#undef HAVE_CPU_M68K

/* Define if the host CPU is a MIPS */
#undef HAVE_CPU_MIPS

/* Define if the host CPU is a PowerPC */
#undef HAVE_CPU_PPC

/* Define if the host CPU is a 64 bit PowerPC */
#undef HAVE_CPU_PPC64

/* Define if the host CPU is a S390 */
#undef HAVE_CPU_S390

/* Define if the host CPU is a SPARC */
#undef HAVE_CPU_SPARC

/* Define if the host CPU is a x86_64 */
#undef HAVE_CPU_X86_64

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#undef HAVE_DCGETTEXT

/* Defined if we have dladdr () */
#undef HAVE_DLADDR

/* Define to 1 if you have the <dlfcn.h> header file. */
#undef HAVE_DLFCN_H

/* Define to 1 if you have the `fgetpos' function. */
#define HAVE_FGETPOS 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#undef HAVE_FSEEKO

/* Define to 1 if you have the `fsetpos' function. */
#define HAVE_FSETPOS 1

/* Define to 1 if you have the `ftello' function. */
#undef HAVE_FTELLO

/* defined if the compiler implements __func__ */
#undef HAVE_FUNC

/* support for features: */
#undef HAVE_EXTERNAL

/* defined if the compiler implements __FUNCTION__ */
#undef HAVE_FUNCTION

/* Define to 1 if you have the `getifaddrs' function. */
#undef HAVE_GETIFADDRS

/* Define to 1 if you have the `getpagesize' function. */
#undef HAVE_GETPAGESIZE

/* Define if the GNU gettext() function is already present or preinstalled. */
#undef HAVE_GETTEXT

/* Define if you have the iconv() function. */
#undef HAVE_ICONV

/* Define if libxml2 is available */
#define HAVE_LIBXML2 1

/* defined if we have makecontext () */
#undef HAVE_MAKECONTEXT

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* defined if the compiler implements __PRETTY_FUNCTION__ */
#undef HAVE_PRETTY_FUNCTION

/* Defined if we have register_printf_function () */
#undef HAVE_PRINTF_EXTENSION

/* Define to 1 if you have the <process.h> header file. */
#define HAVE_PROCESS_H 1

/* Define if RDTSC is available */
#undef HAVE_RDTSC

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <ucontext.h> header file. */
#undef HAVE_UCONTEXT_H

/* defined if unaligned memory access works correctly */
#undef HAVE_UNALIGNED_ACCESS

/* Define to 1 if you have the <unistd.h> header file. */
#undef HAVE_UNISTD_H

/* Define if valgrind should be used */
#undef HAVE_VALGRIND

/* Defined if compiling for Windows */
#define HAVE_WIN32 1

/* the host CPU */
#define HOST_CPU "x86_32"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
#undef NO_MINUS_C_MINUS_O

/* Name of package */
#define PACKAGE "farsight2"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=Farsight"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Farsight2"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Farsight2 0.0.9"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "farsight2"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.0.9"

/* directory where plugins are located */
#undef PLUGINDIR

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* Version number of package */
#define VERSION "0.0.9"

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#undef WORDS_BIGENDIAN
