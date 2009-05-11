DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

SUBDIRS = \
	gnl \
	tests \
	docs \
	m4 \
	common

DIST_SUBDIRS = $(SUBDIRS)

EXTRA_DIST = \
	gnonlin.spec depcomp \
	AUTHORS COPYING NEWS README RELEASE REQUIREMENTS \
	ChangeLog autogen.sh

ACLOCAL_AMFLAGS = -I common/m4 -I m4

DISTCLEANFILES = _stdint.h

include $(top_srcdir)/common/release.mak
include $(top_srcdir)/common/po.mak

check-valgrind:
	cd tests/check && make check-valgrind

if HAVE_CHECK
check-torture:
	cd tests/check && make torture
else
check-torture:
	true
endif

