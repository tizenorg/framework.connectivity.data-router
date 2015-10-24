Name:       data-router
Summary:    Data Router
Version:    0.3.1
Release:    0
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: cmake
BuildRequires: pkgconfig(glib-2.0) >= 2.26
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(capi-network-serial)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires:  model-build-features
Requires(post): /usr/bin/vconftool

%description
Working as a Router for USB communication
For USB serial communication, reads/writes usb node and routes them to Socket client application.

%package test
Summary:    Serial client test application
Group:      TO_BE/FILLED
Requires:   %{name} = %{version}-%{release}

%description test
This package is serial client test application.

%prep
%setup -q

%build

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

install -D -m 0644 data-router.service %{buildroot}%{_libdir}/systemd/system/data-router.service

mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}

%post
#/usr/bin/vconftool set -t int memory/data_router/osp_serial_open "0" -u 0 -i -f -s tizen::vconf::platform::rw

mkdir -p %{_sysconfdir}/systemd/default-extra-dependencies/ignore-units.d/
ln -sf %{_libdir}/systemd/system/data-router.service %{_sysconfdir}/systemd/default-extra-dependencies/ignore-units.d/

/usr/sbin/setcap CAP_CHOWN,CAP_SYS_ADMIN+ep /usr/bin/data-router
%postun

%files
%manifest data-router.manifest
%defattr(-, system, system, -)
/etc/smack/accesses.d/data-router.rule
#%caps(cap_chown,cap_dac_override,cap_lease=eip) %attr(755,root,root) /usr/bin/data-router
%{_bindir}/data-router
%{_libdir}/systemd/system/data-router.service
/usr/share/license/%{name}

%files test
%manifest serial-client-test.manifest
%defattr(-, system, system, -)
%{_bindir}/serial-client-test
