# Generated from testability-driver-0.8.4.gem by gem2rpm -*- rpm-spec -*-
%define ruby_sitelib %(ruby -rrbconfig -e "puts Config::CONFIG['sitelibdir']")
%define gemdir %(ruby -rubygems -e 'puts Gem::dir' 2>/dev/null)
%define gemname testability-driver
%define geminstdir %{gemdir}/gems/%{gemname}-%{version}

Summary: Testability Driver
Name: rubygem-%{gemname}
Version: 0.8.4.20100805091712
Release: 1%{?dist}
Group: Development/Languages
License: GPLv2+ or Ruby
URL: http://code.nokia.com
Source0: %{gemname}-%{version}.gem
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch: noarch
Requires: ruby >= 1.8.7
Requires: rubygems
Requires: rubygem(log4r) >= 1.1.7
Requires: rubygem(nokogiri) >= 1.4.1
Requires: rubygem(builder) = 2.1.2
BuildRequires: rubygems rubygem-log4r rubygem-nokogiri rubygem-rake-compiler rubygem-minitest rubygem-rake rubygem-json_pure
Provides: rubygem(%{gemname}) = %{version}

%description
Testability Driver


%prep

%build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{gemdir}
export TDRIVER_HOME=%{buildroot}/etc/tdriver
export RUBYOPT="rubygems"
gem install --local --install-dir %{buildroot}%{gemdir} \
            --force --rdoc %{SOURCE0}
mkdir -p %{buildroot}/%{_bindir}
mv %{buildroot}%{gemdir}/bin/* %{buildroot}/%{_bindir}
rmdir %{buildroot}%{gemdir}/bin
find %{buildroot}%{geminstdir}/bin -type f | xargs chmod a+x

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root, -)
%{_bindir}/start_app_perf
%{_sysconfdir}/tdriver/*
%{gemdir}/gems/%{gemname}-%{version}/
%doc %{gemdir}/doc/%{gemname}-%{version}
%{gemdir}/cache/%{gemname}-%{version}.gem
%{gemdir}/specifications/%{gemname}-%{version}.gemspec


%changelog
* Tue Aug 03 2010 Tatu Lahtela,,, <ext-tatu.lahtela@nokia.com> - 0.8.4.20100803130204-1
- Initial package
