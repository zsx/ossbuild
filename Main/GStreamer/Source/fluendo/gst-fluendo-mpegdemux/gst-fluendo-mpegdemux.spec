%define majorminor  0.10
%define gstreamer   gstreamer

%define gst_minver  0.10.0

Name:		%{gstreamer}-fluendo-mpegdemux
Version:	0.10.23
Release:	1.flu
Summary:	Fluendo GStreamer plug-in for MPEG demuxing

Group:		Applications/Multimedia
License:	Proprietary
URL:		http://www.fluendo.com/
Source:		gst-fluendo-mpegdemux-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	%{gstreamer}-devel >= %{gst_minver}

%description
Fluendo GStreamer plug-in for MPEG demuxing.

%prep
%setup -q -n gst-fluendo-mpegdemux-%{version}

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

# Remove unneeded .la and .a files
rm -rf $RPM_BUILD_ROOT/%{_libdir}/gstreamer-%{majorminor}/*.a
rm -rf $RPM_BUILD_ROOT/%{_libdir}/gstreamer-%{majorminor}/*.la

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README NEWS
%{_libdir}/gstreamer-%{majorminor}/*.so

%changelog
* Tue Oct 04 2005 Thomas Vander Stichele <thomas at apestaart dot org>
- remove register calls

* Fri Sep 02 2005 Thomas Vander Stichele <thomas at apestaart dot org>
- Clean up spec file

* Fri Jul 08 2005 Christian F.K. Schaller <christian@fluendo.com>
- First attempt at spec
