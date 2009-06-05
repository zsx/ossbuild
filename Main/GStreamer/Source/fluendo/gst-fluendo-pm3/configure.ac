AC_PREREQ(2.52)

dnl please read gstreamer/docs/random/autotools before changing this file

dnl initialize autoconf
dnl fill in your package name and version here
dnl the fourth (nano) number should be 0 for a release, 1 for CVS,
dnl and 2... for a prerelease
dnl releases only do -Wall, cvs and prerelease does -Werror too
AC_INIT(GStreamer Fluendo MP3 Decoder, 0.10.10,,
    gst-fluendo-mp3)

AG_GST_INIT

dnl initialize automake
AM_INIT_AUTOMAKE

dnl define PACKAGE_VERSION_* variables
AS_VERSION

dnl check if this is a release version
AS_NANO(GST_CVS="no", GST_CVS="yes")

dnl define the output header for config
AM_CONFIG_HEADER([config.h])

dnl AM_MAINTAINER_MODE only provides the option to configure to enable it
AM_MAINTAINER_MODE

dnl sets host_* variables
AC_CANONICAL_HOST

dnl make aclocal work in maintainer mode
AC_SUBST(ACLOCAL_AMFLAGS, "-I common/m4")

dnl *** check for arguments to configure ***

AG_GST_ARG_DEBUG
AG_GST_ARG_PROFILING
AG_GST_ARG_VALGRIND
AG_GST_ARG_GCOV

AG_GST_ARG_EXAMPLES

AG_GST_ARG_WITH_PKG_CONFIG_PATH
AG_GST_ARG_WITH_PACKAGE_NAME([Fluendo MP3 Plug-in])
AG_GST_ARG_WITH_PACKAGE_ORIGIN

dnl Header file checks
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h string.h fcntl.h dirent.h sys/ndir.h \
    ndir.h alloca.h locale.h )

dnl *** checks for platform ***

dnl * hardware/architecture *

dnl check CPU type
AG_GST_ARCH

dnl *** checks for programs ***

AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_PATH_PROG(VALGRIND_PATH, valgrind, no)
AM_CONDITIONAL(HAVE_VALGRIND, test ! "x$VALGRIND_PATH" = "xno")

dnl define an ERROR_CFLAGS Makefile variable
AG_GST_SET_ERROR_CFLAGS($GST_CVS)

dnl Check for pkgconfig first
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, yes, no)

dnl Give error and exit if we don't have pkgconfig
if test "x$HAVE_PKGCONFIG" = "xno"; then
  AC_MSG_ERROR(you need to have pkgconfig installed !)
fi

dnl Setup for finding IPP libraries. Attempt to detect by default.
AG_CHECK_IPP
if test "$HAVE_IPP" = "false"; then
  AC_MSG_WARN([Intel Performance Primitives not found in $IPP_PREFIX])
else
  IPP_INCLUDES="-I$IPP_PREFIX/$ver/$IPP_CPU/include"
  IPP_LIBS="-L$IPP_PREFIX/$ver/$IPP_CPU/lib -lippacemerged$IPP_SUFFIX -lippacmerged$IPP_SUFFIX -lippsemerged$IPP_SUFFIX -lippsmerged$IPP_SUFFIX -lippcore$IPP_SUFFIX"
fi
AC_SUBST(IPP_LIBS)
AC_SUBST(IPP_INCLUDES)

HAVE_LIBOIL=false
test_liboil=false

AC_ARG_WITH(liboil,
    AC_HELP_STRING([--with-liboil],
        [Turn on/off use of Liboil (default=no)]),
	[if test "x$withval" = "xyes"; then test_liboil=true; fi])

dnl When building without IPP, we can use liboil
if test "x$test_liboil" = "xtrue"; then
  MIN_LIBOIL=0.3.7
  PKG_CHECK_MODULES(LIBOIL, liboil-0.3 >= $MIN_LIBOIL, HAVE_LIBOIL=yes, HAVE_LIBOIL=no)
  if test "x$HAVE_LIBOIL" = "xyes"; then
    AC_DEFINE(USE_LIBOIL, TRUE, [Build liboil code paths])
  fi
else
AC_MSG_CHECKING([for liboil-0.3])
echo -n "check disabled"
fi
AM_CONDITIONAL(USE_LIBOIL, test "x$HAVE_LIBOIL" = "xtrue")

dnl Now we're ready to ask for gstreamer libs and cflags
dnl And we can also ask for the right version of gstreamer

GST_REQ=0.10.0

GST_MAJORMINOR=0.10

AG_GST_CHECK_GST($GST_MAJORMINOR, [$GST_REQ])

dnl make GST_MAJORMINOR available in Makefile.am
AC_SUBST(GST_MAJORMINOR)

dnl define CPU_TUNE_CFLAGS and CPU_TUNE_CCASFLAGS
AG_GST_CPU_TUNE

AG_GST_SET_PLUGINDIR

if test "x$USE_PROFILING" = "xtrue"; then
  CFLAGS="$CFLAGS -pg"
  LDFLAGS="$LDFLAGS -pg"
fi

dnl Add unroll-loops to the default CFLAGS 
CFLAGS="$CFLAGS -funroll-loops"

AC_SUBST(CFLAGS)

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

AC_OUTPUT([
Makefile
decoder/Makefile
common/Makefile
common/m4/Makefile
util/Makefile
gst-fluendo-mp3.spec
])

echo -n "
$PACKAGE-$GST_PLUGIN_VERSION

        prefix:                         ${prefix}
        compiler:                       ${CC}

        Use Intel Performance Primitives: ${test_ipp}
        IPP CFLAGS:  ${IPP_INCLUDES}
        IPP LDFLAGS: ${IPP_LIBS}
"
if test "x$HAVE_IPP" = "xtrue"; then
echo "        Use Liboil: disabled by IPP"
else
echo "        Use Liboil: ${HAVE_LIBOIL}"
fi
