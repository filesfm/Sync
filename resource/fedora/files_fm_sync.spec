%global debug_package %{nil}

Name:           files-fm-sync
Version:        __VERSION__
Release:        1
Summary:        Files.fm Sync desktop client

License:        GPL-2.0-only
URL:             https://github.com/filesfm/Sync
Source0:        https://github.com/filesfm/Sync/archive/refs/tags/v%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  ninja-build
BuildRequires:  gcc-c++
BuildRequires:  pkgconf-pkg-config
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtdeclarative-devel
BuildRequires:  qt6-qt5compat-devel
BuildRequires:  qt6-qtwebsockets-devel
BuildRequires:  qt6-qtshadertools-devel
BuildRequires:  qt6-qtwebengine-devel
BuildRequires:  qt6-qtwebchannel-devel
BuildRequires:  qt6-qtpositioning-devel
BuildRequires:  openssl-devel
BuildRequires:  sqlite-devel
BuildRequires:  libp11-devel
BuildRequires:  libX11-devel
BuildRequires:  libxkbcommon-devel
BuildRequires:  libxkbcommon-x11-devel
BuildRequires:  libxcb-devel
BuildRequires:  glib2-devel
BuildRequires:  dbus-devel
BuildRequires:  zlib-devel
BuildRequires:  bzip2-devel
BuildRequires:  xz-devel
BuildRequires:  libzstd-devel
BuildRequires:  librsvg2-tools
BuildRequires:  kf6-karchive-devel
BuildRequires:  qt6keychain-devel
Requires:       hicolor-icon-theme

%description
Files.fm Sync keeps a local folder synchronized with your Files.fm
storage, built on the Nextcloud desktop sync engine.

%prep
%autosetup -n Sync-%{version}

%build
cmake -S . -B build/Release -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
  -DBUILD_UPDATER=OFF -DCMAKE_INSTALL_PREFIX=%{_prefix}
cmake --build build/Release

%install
DESTDIR=%{buildroot} cmake --install build/Release

# The desktop file and hicolor icons are already installed by CMake's
# own install() rules; only the AppStream metainfo file still needs
# staging by hand.
install -Dm644 com.files.fm.sync.metainfo.xml \
  %{buildroot}%{_datadir}/metainfo/com.files.fm.sync.metainfo.xml

%files
%license COPYING
%{_bindir}/files_fm_sync
%{_bindir}/files_fm_synccmd
%{_datadir}/applications/com.files.fm.sync.desktop
%{_datadir}/icons/hicolor/*/apps/files_fm_sync.png
%{_datadir}/metainfo/com.files.fm.sync.metainfo.xml

%changelog
