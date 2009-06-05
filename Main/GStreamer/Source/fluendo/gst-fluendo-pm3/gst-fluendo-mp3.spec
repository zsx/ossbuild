%define majorminor  0.10
%define gstreamer   gstreamer

%define gst_minver  0.10.0

Name:		%{gstreamer}-fluendo-mp3
Version:	0.10.10
Release:	1.flu
Summary:	Fluendo GStreamer plug-in for mp3 support

Group:		Applications/Multimedia
License:	MIT
URL:		http://www.fluendo.com/
Source:		gst-fluendo-mp3-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  %{gstreamer}-devel >= %{gst_minver}

%ifarch i386
BuildRequires:  intel-ipp_ia32
%endif

%ifarch x86_64
BuildRequires:  intel-ipp_em64t
%endif

%description
Fluendo GStreamer plug-in for mp3 support.

%prep
%setup -q -n gst-fluendo-mp3-%{version}

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

# Remove unneeded .la and .a files
rm -rf $RPM_BUILD_ROOT/%{_libdir}/gstreamer-%{majorminor}/*.a
rm -rf $RPM_BUILD_ROOT/%{_libdir}/gstreamer-%{majorminor}/*.la

%post
semanage 2>/dev/null >/dev/null
if [ "$?" -eq 1 ]; then
        semanage fcontext -a -t textrel_shlib_t %{_libdir}/gstreamer-%{majorminor}/libgstflump3dec.so
        if [ "$?" -eq 0 ]; then
                echo "Changed local file context in SELinux"
        else
                echo "Failed to run semanage"
        fi
fi
restorecon 2>/dev/null >dev/null
if [ "$?" -eq 1 ]; then
        restorecon %{_libdir}/gstreamer-%{majorminor}/libgstflump3dec.so
        if [ "$?" -eq 0 ]; then
                echo "Changed SELinux permissions for" %{_libdir}/gstreamer-%{majorminor}/libgstflump3dec.so
        else
                echo "Failed to change SELinux permissions for " %{_libdir}/gstreamer-%{majorminor}/libgstflump3dec.so
        fi  
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root, -)
%doc README ChangeLog COPYING AUTHORS NEWS TODO LICENSE
%{_libdir}/gstreamer-%{majorminor}/libgstflump3dec.so

%changelog
* Fri Dec 14 2007 Christophe Eymard <ceymard@fluendo.com>
- persistent settings across reboots and SELinux disabling/reenabling.

* Tue Oct 04 2005 Thomas Vander Stichele <thomas at apestaart dot org>
- removed -register

* Fri Jul 08 2005 Christian F.K. Schaller <christian@fluendo.com>
- First attempt at spec
