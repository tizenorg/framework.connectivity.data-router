Name:       data-router
Summary:    Data Router
Version:    0.2.17
Release:    1
Group:      TO_BE/FILLED_IN
License:    Apache License Version 2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: cmake
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(tapi)
Requires(post): /usr/bin/vconftool

%description
Working as a Router for USB communication
For USB serial communication, reads/writes usb node and routes them to Socket client application.

%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install


%post
/usr/bin/vconftool set -t int memory/data_router/osp_serial_open "0" -u 0 -i -f
%postun


%files
%manifest data-router.manifest
%defattr(-, root, root)
/usr/bin/data-router
