# shellcheck shell=bash
# Fedora row of the README tables. Sourced by smoke-package.sh.

prepare() {
  # The fedora container has no sudo, copr plugin or X server.
  dnf install -y --setopt=install_weak_deps=False \
    sudo dnf-plugins-core xorg-x11-server-Xvfb xorg-x11-xauth
}

install_latest() {
  sudo dnf copr enable -y filesfm/Sync && sudo dnf install -y files-fm-sync
}

install_prelast() {
  sudo dnf copr enable -y filesfm/Sync && sudo dnf install -y "files-fm-sync-$PRELAST"
}

update() {
  sudo dnf upgrade -y files-fm-sync
}

remove() {
  sudo dnf remove -y files-fm-sync && sudo dnf copr disable -y filesfm/Sync
}

installed_version() {
  rpm -q --qf '%{VERSION}-%{RELEASE}' files-fm-sync 2>/dev/null || true
}

is_installed() {
  rpm -q files-fm-sync >/dev/null 2>&1
}

app_path() {
  echo /usr/bin/files_fm_sync
}
