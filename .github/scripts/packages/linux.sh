# shellcheck shell=bash
# Linux (General) row of the README tables: the AppImage. Sourced by smoke-package.sh.
# The README command downloads the newest AppImage into the current directory.
# Removal and the pre-last download are not in the README, so they are defined here.

prepare() {
  sudo apt-get update
  sudo apt-get install -y xvfb xauth libopengl0 libegl1 libxkbcommon0 libxkbcommon-x11-0
  # AppImages need FUSE 2; the package name differs by Ubuntu release.
  sudo apt-get install -y libfuse2t64 || sudo apt-get install -y libfuse2
}

download_latest() {
  curl -sL https://api.github.com/repos/filesfm/Sync/releases/latest \
    | grep -o '"browser_download_url": *"[^"]*\.AppImage"' \
    | head -n1 | cut -d'"' -f4 | xargs -r curl -fL -O
  chmod +x files_fm_sync-*.AppImage
}

install_latest() {
  download_latest
}

install_prelast() {
  gh release download "v$PRELAST" -R filesfm/Sync --pattern "files_fm_sync-$PRELAST.AppImage"
  chmod +x "files_fm_sync-$PRELAST.AppImage"
}

update() {
  download_latest
}

remove() {
  rm -f files_fm_sync-*.AppImage
}

# Update keeps the older AppImage next to the new one, so report the highest version present.
installed_version() {
  local file versions=()
  for file in files_fm_sync-*.AppImage; do
    [ -e "$file" ] || continue
    file=${file#files_fm_sync-}
    versions+=("${file%.AppImage}")
  done
  printf '%s\n' "${versions[@]}" | sort -V | tail -n1
}

is_installed() {
  compgen -G 'files_fm_sync-*.AppImage' >/dev/null
}

app_path() {
  echo "$PWD/files_fm_sync-$(installed_version).AppImage"
}
