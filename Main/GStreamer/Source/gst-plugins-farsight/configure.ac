AC_PREREQ([2.59])
dnl fill in your package name and version here
dnl autoconf configuration files for gst-plugins-farsight
AC_INIT
AC_CANONICAL_TARGET

AC_CONFIG_MACRO_DIR([m4])
dnl We disable static building for development, for time savings
dnl this goes before AS_LIBTOOL to appease autoconf
dnl *NOTE*: dnl this line before release, so release does static too
AM_DISABLE_STATIC

dnl when going to/from release please set the nano (fourth number) right !
dnl releases only do Wall, cvs and prerelease does Werror too
AS_VERSION(gst-plugins-farsight, GST_PLUGINS_VERSION, 0, 12, 11, 0, GST_CVS="no", GST_CVS="yes")

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

AM_PROG_LIBTOOL

AM_CONFIG_HEADER(config.h)

dnl make aclocal work in maintainer mode
AC_SUBST(ACLOCAL_AMFLAGS, "-I m4")

dnl check for tools
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AM_PROG_AS

dnl decide on error flags
AS_COMPILER_FLAG(-Wall, GST_WALL="yes", GST_WALL="no")

if test "x$GST_WALL" = "xyes"; then
  ERROR_CFLAGS="$GST_ERROR -Wall -Wdeclaration-after-statement"

  if test "x$GST_CVS" = "xyes"; then
    AS_COMPILER_FLAG(-Werror,ERROR_CFLAGS="$ERROR_CFLAGS -Werror",ERROR_CFLAGS="$ERROR_CFLAGS")
  fi
fi

AC_SUBST(ERROR_CFLAGS)

dnl determine c++ compiler
AC_PROG_CXX
dnl determine if c++ is available on this system
AC_CHECK_PROG(HAVE_CXX, $CXX, yes, no)
dnl determine c++ preprocessor
AC_PROG_CXXCPP
AC_ISC_POSIX

AC_HEADER_STDC([])
AC_C_INLINE

dnl Check for pkgconfig first
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, yes, no)

dnl Give error and exit if we don't have pkgconfig
if test "x$HAVE_PKGCONFIG" = "xno"; then
  AC_MSG_ERROR(you need to have pkgconfig installed !)
fi

dnl Now we're ready to ask for gstreamer libs and cflags
dnl And we can also ask for the right version of gstreamer

GST_REQUIRED=0.10.11
GST_MAJORMINOR=0.10

PKG_CHECK_MODULES(GST, \
  gstreamer-$GST_MAJORMINOR >= $GST_REQUIRED,
  HAVE_GST=yes,HAVE_GST=no)

dnl Give error and exit if we don't have gstreamer
if test "x$HAVE_GST" = "xno"; then
  AC_MSG_ERROR(you need gstreamer >= $GST_REQUIRED development packages installed !)
fi

if test "x$USE_DEBUG" = xyes; then
  GST_CFLAGS="$GST_CFLAGS -g"
fi

dnl make GST_CFLAGS and GST_LIBS available
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)

dnl make GST_MAJORMINOR available in Makefile.am
AC_SUBST(GST_MAJORMINOR)

dnl check for gstreamer-base; uninstalled is selected preferentially
PKG_CHECK_MODULES(GST_BASE, gstreamer-base-$GST_MAJORMINOR >= $GST_REQUIRED,
  HAVE_GST_BASE="yes", HAVE_GST_BASE="no")

if test "x$HAVE_GST_BASE" = "xno"; then
  AC_MSG_ERROR(no GStreamer Base Libs found)
fi

AC_SUBST(GST_BASE_LIBS)
AC_SUBST(GST_BASE_CFLAGS)

dnl check for gstreamer-libs; uinstalled is selected preferentially
PKG_CHECK_MODULES(GST_PLUGINS_BASE, gstreamer-plugins-base-$GST_MAJORMINOR >= $GST_REQUIRED,
  HAVE_GST_PLUGINS="yes", HAVE_GST_PLUGINS="no")

if test "x$HAVE_GST_PLUGINS" = "xno"; then
  AC_MSG_ERROR(no GStreamer Plugins Libs found)
fi

AC_SUBST(GST_PLUGINS_BASE_LIBS)
AC_SUBST(GST_PLUGINS_BASE_CFLAGS)

dnl set the plugindir where plugins should be installed
plugindir="\$(libdir)/gstreamer-$GST_MAJORMINOR"
AC_SUBST(plugindir)

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\|fs_\|Fs\|FS_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

dnl these are all the gst plug-ins, compilable without additional libs
GST_PLUGINS_ALL="\
rtpdemux \
rtpjitterbuffer \
network-simulator \
rtppayloads \
selector"

AC_SUBST(GST_PLUGINS_ALL)

GST_PLUGINS_SELECTED=""

AC_ARG_WITH(plugins,
AC_HELP_STRING([--with-plugins],
  [comma-separated list of plug-ins to compile]),
  [for i in `echo $withval | tr , ' '`; do
    if echo $GST_PLUGINS_ALL | grep $i > /dev/null
    then
      GST_PLUGINS_SELECTED="$GST_PLUGINS_SELECTED $i"
    else
      echo "plug-in $i not recognized, ignoring..."
    fi
  done],
[GST_PLUGINS_SELECTED=$GST_PLUGINS_ALL])

AC_SUBST(GST_PLUGINS_SELECTED)

dnl check for jrtplib; prefer to _not_ use it
AC_ARG_ENABLE(jrtplib,
  AC_HELP_STRING([--enable-jrtplib],
    [compile with jrtplib support (Not recommended!)]),
  try_jrtplib=$enableval, try_jrtplib=no)

HAVE_JRTP_JTHREAD=no
if test x$try_jrtplib = xyes ; then
  JRTPLIB_REQUIRED=3.3.0
  PKG_CHECK_MODULES(JRTP, jrtplib >= $JRTPLIB_REQUIRED,
    HAVE_JRTP="yes", HAVE_JRTP="no")
  AC_SUBST(JRTP_LIBS)
  AC_SUBST(JRTP_CFLAGS)
  JTHREAD_REQUIRED=1.1.2
  PKG_CHECK_MODULES(JTHREAD, jthread >= $JTHREAD_REQUIRED,
    HAVE_JTHREAD="yes", HAVE_JTHREAD="no")
  AC_SUBST(JTHREAD_LIBS)
  AC_SUBST(JTHREAD_CFLAGS)
  AC_MSG_RESULT(Found libjrtp... $HAVE_JRTP)
  AC_MSG_RESULT(Found libjthread... $HAVE_JTHREAD)
  if test "x$HAVE_JRTP" = "xyes" ; then
    if test "x$HAVE_JTHREAD" = "xyes" ; then
      AC_DEFINE(HAVE_JRTP, , [Use jrtplib])
      AM_CONDITIONAL(USE_JRTP, true)
      HAVE_JRTP_JTHREAD=yes
    fi
  fi
fi
if test x$HAVE_JRTP_JTHREAD = xyes ; then
  AC_MSG_WARN([Using Jrtplib, this is _NOT_ supported.])
else
  AM_CONDITIONAL(USE_JRTP, false)
fi

dnl check for gconf
translit(dnm, m, l) AM_CONDITIONAL(USE_GCONF, true)
GST_CHECK_FEATURE(GCONF, [gconf library], gconf, [
  GCONF_REQUIRED=2.4
  PKG_CHECK_MODULES(GCONF, \
      gconf-2.0 >= $GCONF_REQUIRED,
      HAVE_GCONF=yes,HAVE_GCONF=no)
  AC_SUBST(GCONF_CFLAGS)
  AC_SUBST(GCONF_LIBS)
  ])
AC_MSG_RESULT(Found libgconf... $HAVE_GCONF)

dnl Check for GSM
HAVE_GSM="no"
translit(dnm, m, l) AM_CONDITIONAL(USE_GSM, true)
GST_CHECK_FEATURE(GSM, [GSM library], gsmenc gsmdec, [
  GST_CHECK_LIBHEADER(GSM, gsm, gsm_create, , gsm.h, GSM_LIBS="-lgsm" HAVE_GSM="yes")
  if test "x$HAVE_GSM" != "xyes"; then
    GST_CHECK_LIBHEADER(GSM, gsm, gsm_create, , gsm/gsm.h, GSM_LIBS="-lgsm" HAVE_GSM="yes")
    if test "x$HAVE_GSM" == "xyes"; then 
      AC_DEFINE(GSM_HEADER_IN_SUBDIR, 1, [Define if GSM header in gsm/ subdir])
    fi
  fi
  AC_SUBST(GSM_LIBS)
])
AC_MSG_RESULT(checking for libgsm... $HAVE_GSM)

dnl Check libgsm has WAV49 capability
if test "x$HAVE_GSM" == "xyes"; then 
  ac_test_save_LIBS=$LIBS
  LIBS="-lgsm $LIBS"
  AC_RUN_IFELSE([
    AC_LANG_PROGRAM(
      [[ #if GSM_HEADER_IN_SUBDIR
           #include <gsm/gsm.h>
         #else 
           #include <gsm.h>
         #endif
         gsm r; 
         int tmp; 
      ]],
      [[ 
        if (!(r = gsm_create())) 
          return 1;
        if (gsm_option(r, GSM_OPT_WAV49,   &tmp) == -1) 
          return 1;
        else 
          return 0;
      ]])],
    [
      AC_DEFINE(GSM_HAS_WAV49, 1, [Define if libgsm has suport for WAV49.])
      AM_CONDITIONAL(USE_GSM_WAV49, true)
      AC_MSG_RESULT(checking libgsm has WAV49 mode... yes)
    ],
    [
      AM_CONDITIONAL(USE_GSM_WAV49, false)
      AC_MSG_RESULT(Checking libgsm has WAV49 mode... no)
    ])
  LIBS=$ac_test_save_LIBS
else
  AM_CONDITIONAL(USE_GSM_WAV49, false)
fi

dnl libj2k
AC_ARG_WITH([libj2k],
  AC_HELP_STRING([--with-libj2k=PREFIX],
      [Compile the libj2k codec plugin, using the libj2k library]),
  [libj2k="$withval"],
  [libj2k="no"])

AC_MSG_RESULT(Checking for --with-libj2k... $libj2k)
AM_CONDITIONAL(USE_LIBJ2K, test "x$libj2k" != "xno")

dnl if entered yes use /usr default value
if test "x$libj2k" = "xyes" ; then
  libj2k="/usr"
fi

dnl set the given prefix
if test "x$libj2k" != "xno" ; then
  LIBJ2K_PREFIX=$libj2k
else
  dnl AC_CHECK_LIB([j2k], j2k_encode, enable_libj2k=yes, enablelibj2k=no, -lm)
  LIBJ2K_PREFIX=
fi

AC_SUBST(LIBJ2K_PREFIX)

dnl libjasper
translit(dnm, m, l) AM_CONDITIONAL(USE_LIBJASPER, true)
GST_CHECK_FEATURE(JASPER, [JASPER library], jasperenc, [
  GST_CHECK_LIBHEADER(JASPER, jasper, jas_stream_fopen, , jasper/jasper.h, JASPER_LIBS="-ljasper")
  AC_SUBST(JASPER_LIBS)
])
AC_MSG_RESULT(checking for libjasper... $HAVE_JASPER)

dnl jingle
AC_ARG_ENABLE(jingle-p2p,
  AC_HELP_STRING([--disable-jingle-p2p],[compile without jingle-p2p support]),
  try_jinlge_p2p=$enableval, try_jinlge_p2p=yes)

HAVE_JINGLEP2P=no
if test x$try_jinlge_p2p = xyes ; then
  LIBJINGLE_MAJORMINOR=0.3
  dnl check for libjingle
  PKG_CHECK_MODULES(JINGLEBASE, jinglebase-$LIBJINGLE_MAJORMINOR,
    [
      PKG_CHECK_MODULES(JINGLEP2P, jinglep2p-$LIBJINGLE_MAJORMINOR,
        [
        HAVE_JINGLEP2P=yes
        AC_SUBST(JINGLEBASE_CFLAGS)
        AC_SUBST(JINGLEBASE_LIBS)
        AC_SUBST(JINGLEP2P_CFLAGS)
        AC_SUBST(JINGLEP2P_LIBS)
        ],
        [
        HAVE_JINGLEP2P=no
        AC_MSG_WARN([Jingle-p2p $LIBJINGLE_MAJORMINOR not found, some plugins may be disabled.])
        ])
    ],
    [ 
    HAVE_JINGLEP2P=no
    AC_MSG_WARN([Jingle-base $LIBJINGLE_MAJORMINOR not found, some plugins may be disabled.])
    ])
else
  AC_MSG_NOTICE([Jingle usage disabled, some plugins may be disabled.])
fi

if test x$HAVE_JINGLEP2P = xno; then
  USE_JINGLEP2P=no
else
  USE_JINGLEP2P=yes
fi

AC_SUBST(HAVE_JINGLEP2P)
AM_CONDITIONAL(USE_JINGLEP2P, test x$USE_JINGLEP2P = xyes)

AC_OUTPUT(Makefile \
ext/Makefile \
ext/jrtp/Makefile \
ext/gsm/Makefile \
ext/jpeg2000/Makefile \
ext/jasper/Makefile \
ext/ice/Makefile \
ext/gconf/Makefile \
gst/Makefile \
gst/rtppayloads/Makefile \
gst/rtpdemux/Makefile \
gst/rtpjitterbuffer/Makefile \
gst/network-simulator/Makefile \
gst/selector/Makefile \
tests/Makefile \
m4/Makefile
)
