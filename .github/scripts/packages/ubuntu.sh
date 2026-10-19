# shellcheck shell=bash
# Ubuntu row of the README tables. Sourced by smoke-package.sh.

prepare() {
  sudo apt-get update
  sudo apt-get install -y xvfb xauth
}

install_latest() {
  sudo add-apt-repository -y ppa:filesfm/sync && sudo apt update && sudo apt install -y files-fm-sync
}

install_prelast() {
  sudo add-apt-repository -y ppa:filesfm/sync && sudo apt update && sudo apt install -y "files-fm-sync=$PRELAST"
}

update() {
  sudo apt update && sudo apt upgrade -y files-fm-sync
}

remove() {
  sudo apt remove -y files-fm-sync && sudo add-apt-repository -y --remove ppa:filesfm/sync
}

installed_version() {
  # shellcheck disable=SC2016 # dpkg-query format string, not a shell expansion
  dpkg-query -W -f='${Version}' files-fm-sync 2>/dev/null || true
}

is_installed() {
  # shellcheck disable=SC2016
  dpkg-query -W -f='${Status}' files-fm-sync 2>/dev/null | grep -q '^install ok installed$'
}

app_path() {
  echo /usr/bin/files_fm_sync
}
