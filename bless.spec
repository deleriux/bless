Name: bless
Version: 1.0
Release: 3%{?dist}
Summary: Program to allow users to have ambient capabilities
Source0: bless-1.0.tar.gz

License: GPLv2+
BuildRequires: libcap-devel
Requires: libcap

%description
Program to allow users to have ambient capabilities or curse users never
to exceed their privilege level.

%prep
%setup -q

%build
./configure --prefix=$RPM_BUILD_ROOT/usr
make

%install
make install

%post
/usr/sbin/setcap all=i /usr/bin/bless

%files
%defattr(-,root,root,-)
%{_bindir}/bless
%{_bindir}/curse

%clean
rm -rf %{buildroot}

%changelog
* Thu May 20 2021 Matthew Ife <matthew@ife.onl> - 1.0-3
- Fix setcap command in makefile so rpm works.
- Add requires.

* Thu May 20 2021 Matthew Ife <matthew@ife.onl> - 1.0-1
- Initial release.
