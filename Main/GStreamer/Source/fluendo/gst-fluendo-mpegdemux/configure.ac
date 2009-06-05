AC_PREREQ(2.52)

dnl please read gstreamer/docs/random/autotools before changing this file

dnl initialize autoconf
dnl fill in your package name and version here
dnl the fourth (nano) number should be 0 for a release, 1 for CVS,
dnl and 2... for a prerelease
dnl releases only do -Wall, cvs and prerelease does -Werror too
AC_INIT(GStreamer Fluendo MPEG Demuxer, 0.10.23,,
    gst-fluendo-mpegdemux)

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

AG_GST_ARG_PROFILING
AG_GST_ARG_VALGRIND
AG_GST_ARG_GCOV

dnl *** checks for programs ***

AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

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

dnl Check for liboil
HAVE_LIBOIL=false
test_liboil=false

AC_ARG_WITH(liboil,
    AC_HELP_STRING([--with-liboil],
        [Turn on/off use of Liboil (default=no)]),
	[if test "x$withval" = "xyes"; then test_liboil=true; fi])

if test "x$test_liboil" = "xtrue"; then
  MIN_LIBOIL=0.3.7
  PKG_CHECK_MODULES(LIBOIL, liboil-0.3 >= $MIN_LIBOIL, HAVE_LIBOIL=yes, HAVE_LIBOIL=no)
  if test "x$HAVE_LIBOIL" = "xyes"; then
    AC_DEFINE(USE_LIBOIL, TRUE, [Build liboil code paths])
  fi
fi
AM_CONDITIONAL(USE_LIBOIL, test "x$HAVE_LIBOIL" = "xtrue")

dnl Now we're ready to ask for gstreamer libs and cflags
dnl And we can also ask for the right version of gstreamer

GST_REQ=0.10.3
GSTPB_REQ=0.10.3

GST_MAJORMINOR=0.10

AG_GST_CHECK_GST($GST_MAJORMINOR, [$GST_REQ])
AG_GST_CHECK_GST_BASE($GST_MAJORMINOR, [$GST_REQ])

GST_CFLAGS="$GST_CFLAGS $ERROR_CFLAGS"

dnl make GST_CFLAGS and GST_LIBS available
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)
AC_SUBST(GST_BASE_LIBS)

dnl make GST_MAJORMINOR available in Makefile.am
AC_SUBST(GST_MAJORMINOR)

AG_GST_SET_PLUGINDIR

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

AC_OUTPUT(
Makefile
src/Makefile
common/Makefile
common/m4/Makefile
gst-fluendo-mpegdemux.spec
)
