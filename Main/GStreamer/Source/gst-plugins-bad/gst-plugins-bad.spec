%define majorminor  0.10
%define gstreamer   gstreamer

%define gst_minver   0.10.0

Name: 		%{gstreamer}-plugins-bad
Version: 	0.10.10
Release: 	1.gst
Summary: 	GStreamer plug-ins of bad quality

%define 	majorminor	0.10

Group: 		Applications/Multimedia
License: 	LGPL
URL:		http://gstreamer.freedesktop.org/
Vendor:         GStreamer Backpackers Team <package@gstreamer.freedesktop.org>
Source:         http://gstreamer.freedesktop.org/src/gst-plugins-bad/gst-plugins-bad-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires:         %{gstreamer} >= %{gst_minver}
BuildRequires:    %{gstreamer}-devel >= %{gst_minver}

BuildRequires:  gcc-c++
BuildRequires: ladspa-devel
BuildRequires:  faad2-devel >= 2.0
BuildRequires:  gsm-devel >= 1.0.10
Requires:      SDL >= 1.2.0
#BuildRequires:  swfdec-devel
Provides:      gstreamer-faad = %{version}-%{release}
Requires:      faac >= 1.23
Provides:       gstreamer-gsm = %{version}-%{release}
Requires: libmms >= 0.1
Requires: gmyth
Requires: x264

%description
GStreamer is a streaming media framework, based on graphs of filters which
operate on media data. Applications using this library can do anything
from real-time sound processing to playing videos, and just about anything
else media-related.  Its plugin-based architecture means that new data
types or processing capabilities can be added simply by installing new
plug-ins.

This package contains GStreamer Plugins that are considered to be of bad
quality, even though they might work.

%prep
%setup -q -n gst-plugins-bad-%{version}

%build
%configure --enable-experimental

make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT

%makeinstall
                                                                                
# Clean out files that should not be part of the rpm.
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.la
rm -f $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.a
rm -f $RPM_BUILD_ROOT%{_libdir}/*.la

%find_lang gst-plugins-bad-%{majorminor}

%clean
rm -rf $RPM_BUILD_ROOT

%files -f gst-plugins-bad-%{majorminor}.lang
%defattr(-, root, root)
%doc AUTHORS COPYING README REQUIREMENTS gst-plugins-bad.doap

# non-core plugins without external dependencies
%{_libdir}/gstreamer-%{majorminor}/libgsttta.so
%{_libdir}/gstreamer-%{majorminor}/libgstspeed.so
%{_libdir}/gstreamer-%{majorminor}/libgstcdxaparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstfreeze.so
%{_libdir}/gstreamer-%{majorminor}/libgstmodplug.so
%{_libdir}/gstreamer-%{majorminor}/libgsth264parse.so
%{_libdir}/gstreamer-%{majorminor}/libgstfilter.so
%{_libdir}/gstreamer-%{majorminor}/libgstnsf.so
%{_libdir}/gstreamer-%{majorminor}/libgstdeinterlace.so
%{_libdir}/gstreamer-%{majorminor}/libgstnuvdemux.so
%{_libdir}/gstreamer-%{majorminor}/libgsty4menc.so
%{_libdir}/gstreamer-%{majorminor}/libgstrfbsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstreal.so
%{_libdir}/gstreamer-%{majorminor}/libgstmve.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegvideoparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstrtpmanager.so
%{_libdir}/gstreamer-%{majorminor}/libgstbayer.so
%{_libdir}/gstreamer-%{majorminor}/libgstdvdspu.so
%{_libdir}/gstreamer-%{majorminor}/libgstfestival.so
%{_libdir}/gstreamer-%{majorminor}/libgststereo.so
%{_libdir}/gstreamer-%{majorminor}/libgstvcdsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstdvb.so
%{_libdir}/gstreamer-%{majorminor}/libgstsdpelem.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpeg4videoparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstfbdevsink.so
%{_libdir}/gstreamer-%{majorminor}/libgstrawparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmetadata.so
%{_libdir}/gstreamer-%{majorminor}/libgstselector.so
%{_libdir}/gstreamer-%{majorminor}/libgstsubenc.so
%{_libdir}/gstreamer-%{majorminor}/libgstoss4audio.so
%{_libdir}/gstreamer-%{majorminor}/libgstdeinterlace2.so
%{_libdir}/gstreamer-%{majorminor}/libresindvd.so
%{_libdir}/gstreamer-%{majorminor}/libgstaiffparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstdccp.so
%{_libdir}/gstreamer-%{majorminor}/libgstpcapparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegtsmux.so
%{_libdir}/gstreamer-%{majorminor}/libgstscaletempoplugin.so
%{_libdir}/gstreamer-%{majorminor}/libgstmpegdemux.so
%{_libdir}/gstreamer-%{majorminor}/libgstflv.so
%{_libdir}/gstreamer-%{majorminor}/libgstjp2k.so
%{_libdir}/gstreamer-%{majorminor}/libgstapexsink.so
%{_libdir}/gstreamer-%{majorminor}/libgstaacparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstamrparse.so
%{_libdir}/gstreamer-%{majorminor}/libgstqtmux.so
%{_libdir}/gstreamer-%{majorminor}/libgstlegacyresample.so
%{_libdir}/gstreamer-%{majorminor}/libgstmxf.so
%{_includedir}/gstreamer-%{majorminor}/gst/app/gstappbuffer.h
%{_includedir}/gstreamer-%{majorminor}/gst/app/gstappsink.h
%{_includedir}/gstreamer-%{majorminor}/gst/app/gstappsrc.h
%{_libdir}/gstreamer-%{majorminor}/libgstapp.so
%{_libdir}/gstreamer-%{majorminor}/libgstvmnc.so
%{_libdir}/gstreamer-%{majorminor}/libgstvideosignal.so
%{_libdir}/libgstapp-0.10.so
%{_libdir}/libgstapp-0.10.so.0
%{_libdir}/libgstapp-0.10.so.0.0.0

# gstreamer-plugins with external dependencies but in the main package
%{_libdir}/gstreamer-%{majorminor}/libgstfaad.so
%{_libdir}/gstreamer-%{majorminor}/libgstfaac.so
%{_libdir}/gstreamer-%{majorminor}/libgsttrm.so
%{_libdir}/gstreamer-%{majorminor}/libgstsdl.so
#%{_libdir}/gstreamer-%{majorminor}/libgstswfdec.so
%{_libdir}/gstreamer-%{majorminor}/libgstmms.so
%{_libdir}/gstreamer-%{majorminor}/libgstxvid.so
%{_libdir}/gstreamer-%{majorminor}/libgstbz2.so
#%{_libdir}/gstreamer-%{majorminor}/libgstivorbis.so
%{_libdir}/gstreamer-%{majorminor}/libgstneonhttpsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstalsaspdif.so
%{_libdir}/gstreamer-%{majorminor}/libgstmusepack.so
%{_libdir}/gstreamer-%{majorminor}/libgstgsm.so
%{_libdir}/gstreamer-%{majorminor}/libgstdtsdec.so
%{_libdir}/gstreamer-%{majorminor}/libgstladspa.so
%{_libdir}/gstreamer-%{majorminor}/libgstmythtvsrc.so
%{_libdir}/gstreamer-%{majorminor}/libgstx264.so
%{_libdir}/gstreamer-%{majorminor}/libgstdc1394.so
#%{_libdir}/gstreamer-%{majorminor}/libgsttimidity.so
%{_libdir}/gstreamer-%{majorminor}/libgstwildmidi.so

%changelog
* Thu Oct 9 2008 Christian Schaller <chrisian.schaller at collabora dot co uk>
- flacparse, flvmux and j2kdec plugins added

* Mon Sep 1 2008 Christian Schaller <christian.schaller at collabora dot co uk>
- Add tsmux and scaletempo plugins

* Fri May 2 2008 Christian Schaller <christian.schaller at collabora dot co uk>
- Add Wildmidi plugin

* Mon Apr 14 2008 Tim-Philipp Müller <tim.muller at collabora dot co uk>
- Remove souphttpsrc plugin, which has moved to gst-plugins-good.

* Thu Apr 3 2008 Christian Schaller <christian.schaller at collabora dot co uk>
- Add new OSSv4 plugin to SPEC file

* Tue Apr 1 2008 Tim-Philipp Müller <tim.muller at collabora dot co uk>
- Update spec file for srtenc plugin rename to subenc

* Tue Apr 1 2008 Christian Schaller <christian.schaller at collabora dot co uk>
- Update spec with libgstsrtenc plugin

* Wed Jan 23 2008 Christian Schaller <christian.schaller at collabora dot co uk>
- Update spec with fbdev sink and rawparse, remove videoparse

* Fri Dec 14 2007 Christian Schaller <christian.schaller at collabora dot co uk>
- Update spec file with timidity, libgstdvb, libgstsdpelem, libgstspeexresample, libgstmpeg4videoparse

* Tue Jun 12 2007 Jan Schmidt <jan at fluendo dot com>
- wavpack and qtdemux have moved to good.

* Thu Mar 22 2007 Christian Schaller <christian at fluendo dot com>
- Add x264 and mpegvideoparse plugins

* Fri Dec 15 2006 Thomas Vander Stichele <thomas at apestaart dot org>
- add doap file
- more cleanup

* Sun Nov 27 2005 Thomas Vander Stichele <thomas at apestaart dot org>
- redone for split

