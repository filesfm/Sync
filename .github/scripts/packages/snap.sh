# shellcheck shell=bash
# Snap row of the README tables. Sourced by smoke-package.sh.
# The Snap Store's public API exposes no version history, so only
# install-latest and remove are exercised here (no update/prelast cycle;
# smoke-package.sh runs a reduced lifecycle for packages with no install_prelast).

prepare() {
  sudo apt-get update
  sudo apt-get install -y xvfb xauth
  sudo snap wait system seed.loaded
}

install_latest() {
  sudo snap install files-fm-sync
}

update() {
  sudo snap refresh files-fm-sync
}

remove() {
  sudo snap remove files-fm-sync
}

installed_version() {
  snap list files-fm-sync 2>/dev/null | awk 'NR==2{print $2}'
}

is_installed() {
  snap list files-fm-sync >/dev/null 2>&1
}

app_path() {
  echo /snap/bin/files-fm-sync
}
