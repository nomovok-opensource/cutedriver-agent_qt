Name: qttas-server
Version: 0.9.1
Release:1%{?dist}
Summary: Qt Test Automation Server
Group: Development/Tools
License: LGPL
URL: https://code.nokia.com/
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  pkgconfig(QtCore)
BuildRequires:  pkgconfig(QtWebKit)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xtst)
Requires: qttas-server-plugins

%description
Qt TAS is a test automation server which provides testability 
interface. Interface is used to access UI components to 
verify and control them. 

%prep
%setup -q

%build
# TODO add CONFIG+=maemo
qmake -r CONFIG+=RPM
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make install INSTALL_ROOT=%{buildroot}



%clean
rm -rf %{buildroot}


#
# Package qttas-server
#

%files
%defattr(-,root,root,-)
%{_bindir}/qttas*
%{_sysconfdir}/qt_testability/*
%{_sysconfdir}/xdg/autostart/qttasserver.desktop


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
Requires: qttas-server-libs

%description devel
Qt TAS development header.

%files devel
%defattr(-,root,root,-)
%{_includedir}/*
/usr/share/qt4/mkspecs/features/*
%doc /usr/share/doc/qttas-dev/examples/hellotraverse/*



%package plugins
Summary: Qt Test automation server plugins
%description plugins
Qt Test Automation Server plugins


%files plugins
%defattr(-,root,root,-)
%{_libdir}/qt4/plugins/*


%package -n testability-driver
Summary: Meta package to install the Testability Driver environment.
Requires: qttas-server-libs qttas-server qttas-server-plugins
Requires: rubygem-testability-driver rubygem-testability-driver-qt-sut-plugin
%description -n testability-driver
Qt TAS is a test automation server which provides testability 
interface. Interface is used to access UI components to 
verify and control them. This the metapackage that installs all necessary components.


