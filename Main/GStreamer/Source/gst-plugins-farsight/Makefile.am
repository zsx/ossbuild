ACLOCAL_AMFLAGS = -I m4

SUBDIRS = ext gst
DIST_SUBDIRS = ext gst m4 tests

dist-hook:
	chmod u+w ${distdir}/ChangeLog
	if test -d _darcs; then darcs changes >${distdir}/ChangeLog; fi
