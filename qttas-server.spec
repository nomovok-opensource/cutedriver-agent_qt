Name: qttas-server
Version: 0.9.1
Release: 2%{?dist}
Summary: Qt Test Automation Server
Group: Development/Tools
License: LGPL
Source0: %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Declarative)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5Test)
Requires: qttas-server-plugins

%description
Qt TAS is a test automation server which provides testability 
interface. Interface is used to access UI components to 
verify and control them. 

%prep
%setup -q

%build
qtchooser -run-tool=qmake -qt=5 -r CONFIG+=RPM CONFIG+=no_mobility CONFIG+=no_webkit CONFIG+=wayland
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make install INSTALL_ROOT=%{buildroot}

# remove executable bits from examples
find ${buildroot} -name "*examples*" -type f|xargs chmod a-x

%clean
rm -rf %{buildroot}


#
# Package qttas-server
#

%files
%defattr(-,root,root,-)
%{_bindir}/qttas*
%config %{_sysconfdir}/qt_testability/*
%config %{_sysconfdir}/xdg/autostart/qttasserver.desktop


%package libs
Summary: Qt Test automation server library files

%description libs
Qt TAS development library files

%files libs
%defattr(-,root,root,-)
%{_libdir}/lib*

# other
%post libs 
/sbin/ldconfig
%postun libs 
/sbin/ldconfig


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
%{_includedir}/tdriver/*
/usr/share/qt5/mkspecs/features/*
/usr/share/doc/qttas-dev/examples/hellotraverse/*



%package plugins
Summary: Qt Test automation server plugins
%description plugins
Qt Test Automation Server plugins


%files plugins
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/*

