# shellcheck shell=bash
# Windows row of the README tables. Sourced by smoke-package.sh (runs in Git Bash).

prepare() {
  :
}

install_latest() {
  choco install -y files-fm-sync
}

install_prelast() {
  choco install -y files-fm-sync --version "$PRELAST"
}

update() {
  choco upgrade -y files-fm-sync
}

remove() {
  choco uninstall -y files-fm-sync
}

installed_version() {
  choco list --local-only --exact --limit-output files-fm-sync 2>/dev/null | cut -d'|' -f2 || true
}

is_installed() {
  choco list --local-only --exact --limit-output files-fm-sync 2>/dev/null | grep -q '^files-fm-sync|'
}

app_path() {
  # The WiX MSI installs to $PROGRAMFILES64\Files.fm Sync 2027 (admin/win/msi/Nextcloud.wxs).
  # shellcheck disable=SC2154 # PROGRAMFILES is set by the Windows runner
  echo "$(cygpath -u "$PROGRAMFILES")/Files.fm Sync 2027/files_fm_sync.exe"
}
