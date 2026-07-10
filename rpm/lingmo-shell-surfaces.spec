Name:           lingmo-shell-surfaces
Version:        1.0.0
Release:        1%{?dist}
Summary:        Lingmo Desktop shell surfaces library

License:        LGPLv2.1+
URL:            https://github.com/Matrinsoft/lingmo-shell-surfaces
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.21
BuildRequires:  ninja-build
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qtwayland-devel
BuildRequires:  wayland-devel
BuildRequires:  libxcb-devel
BuildRequires:  libX11-devel
BuildRequires:  lingmo-utils-devel
BuildRequires:  lingmo-config-devel
BuildRequires:  lingmo-ipc-devel
BuildRequires:  lingmo-plugin-devel
BuildRequires:  lingmo-theme-devel
BuildRequires:  lingmo-window-devel
BuildRequires:  lingmo-shell-core-devel
BuildRequires:  lingmo-icon-devel

%description
LingmoShellSurfaces provides the visual shell layer for Lingmo Desktop,
including Panel (top/bottom taskbar), Desktop (wallpaper/background), and
Overview (window switcher/task view) surfaces built on wlr-layer-shell
(Wayland) and EWMH (X11).

%package libs
Summary:        Lingmo Desktop shell surfaces shared library
%description libs
Shared library for LingmoShellSurfaces.

%package devel
Summary:        Lingmo Desktop shell surfaces development files
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       qt6-qtbase-devel
%description devel
Development headers and CMake modules for LingmoShellSurfaces.

%package -n qml-module-lingmo-shell
Summary:        Lingmo Shell QML plugin
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
%description -n qml-module-lingmo-shell
QML bindings for LingmoShellSurfaces providing Lingmo.Shell 1.0 types.

%prep
%autosetup

%build
%cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DBUILD_TESTING=OFF
%cmake_build

%install
%cmake_install

%files libs
%license LICENSES/LGPL-2.1-or-later.txt
%{_libdir}/libLingmoShellSurfaces.so.1
%{_libdir}/libLingmoShellSurfaces.so.%{version}

%files devel
%{_includedir}/LingmoShellSurfaces/
%{_libdir}/libLingmoShellSurfaces.so
%{_libdir}/cmake/LingmoShellSurfaces/

%files -n qml-module-lingmo-shell
%{_qt6_qmldir}/Lingmo/Shell/

%changelog
* Thu Jul 10 2026 Matrinsoft <support@matrinsoft.cn> - 1.0.0-1
- Initial release
