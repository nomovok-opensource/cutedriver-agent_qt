Name: qttas-server
Version: 0.8.4
Release:1%{?dist}
Summary: Qt Test Automation Server
Group: Development/Tools
License: LGPL
URL: https://code.nokia.com/
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: qt-devel libXtst-devel
Requires: qt


%description
Qt TAS is a test automation server which provides testability 
interface. Interface is used to access UI components to 
verify and control them. 

%prep
%setup -q

%build
# TODO add CONFIG+=maemo
qmake -r 
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make install INSTALL_ROOT=%{buildroot}
# For some reason rpmlint doesn't like this file?
rm -rv %{buildroot}/usr/lib/libqttestability.so


%clean
rm -rf %{buildroot}


#
# Package qttas-server
#

%files
%defattr(-,root,root,-)
%{_bindir}/qttas*
%{_sysconfdir}/init/*
%{_sysconfdir}/qt_testability/*
%{_sysconfdir}/X11/*



%package libs
Summary: Qt Test automation server library files

%description libs
Qt TAS development library files

%files libs
%defattr(-,root,root,-)
%{_libdir}/lib*

# other
%post libs -p /sbin/ldconfig
%postun libs -p /sbin/ldconfig


#
# Package qttas-server-devel
#

%package devel
Summary: Qt Test automation server development headers

%description devel
Qt TAS development header.

%files devel
%defattr(-,root,root,-)
%{_includedir}/*




%package plugins
Summary: Qt Test automation server plugins
%description plugins
Qt Test Automation Server plugins

%files plugins
%defattr(-,root,root,-)
%{_libdir}/qt4/plugins/*


%changelog
* Thu Jun 17 2010 - ext-tatu.lahtela@nokia.com - 0.8.4
- Testing RPM Packaging
