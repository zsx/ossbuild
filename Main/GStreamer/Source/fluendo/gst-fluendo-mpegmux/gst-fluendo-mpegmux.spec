%define majorminor  0.10
%define gst_minver  0.10.0

%{!?gstreamer:  %define         gstreamer       gstreamer}

Name:		gstreamer-fluendo-mpegmux
Version:	0.10.4
Release:	1.flu
Summary:	Fluendo GStreamer plug-in for MPEG Transport Stream muxing

Group:		Applications/Multimedia
License:	GPL/LGPL/MPL/MIT
URL:		http://www.fluendo.com/
Source:		gst-fluendo-mpegmux-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  pkgconfig
BuildRequires:  %{gstreamer}-devel
BuildRequires:  %{gstreamer}-plugins-base-devel

%description
Fluendo GStreamer plug-in for MPEG Transport Stream muxing

%prep
%setup -q -n gst-fluendo-mpegmux-%{version}

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

# Remove unneeded .la and .a files
rm -rf $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.a
rm -rf $RPM_BUILD_ROOT%{_libdir}/gstreamer-%{majorminor}/*.la

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README NEWS COPYING COPYING.LGPL COPYING.GPL COPYING.MIT COPYING.MPL
%{_libdir}/gstreamer-%{majorminor}/*.so

%changelog
* Mon Nov 13 2006 Christian F.K. Schaller <christian@fluendo.com>
- First attempt at spec
