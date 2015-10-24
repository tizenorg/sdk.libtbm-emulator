Name:           libtbm-emulator
Version:        0.1.3
Release:        1
License:        MIT
Summary:        Tizen Buffer Manager - emulator backend
Group:          System/Libraries
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig(pthread-stubs)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libtbm)

Requires:   libtbm
Requires:   libdrm2

%description
description: ${summary}

%prep
%setup -q

%build
autoreconf -vfi
./configure --prefix=%{_prefix} --libdir=%{_libdir}/bufmgr \
            CFLAGS="${CFLAGS} -Wall -Werror" LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--as-needed"

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp -af COPYING %{buildroot}/usr/share/license/%{name}

%make_install

%post
if [ -f %{_libdir}/bufmgr/libtbm_default.so ]; then
    rm -rf %{_libdir}/bufmgr/libtbm_default.so
fi
ln -s libtbm_emulator.so %{_libdir}/bufmgr/libtbm_default.so

%postun -p /sbin/ldconfig

%files
%{_libdir}/bufmgr/libtbm_*.so*
/usr/share/license/%{name}
