# shellcheck shell=bash
# macOS row of the README tables. Sourced by smoke-package.sh.
prepare() {
  :
}

install_latest() {
  brew install --cask filesfm/macos/files-fm-sync
}

install_prelast() {
  # Homebrew cannot pin a cask version and refuses casks outside a tap, so the tapped
  # cask file is replaced with the one from PRELAST_REF for this install, then restored.
  local tap
  tap=$(brew --repo filesfm/macos)
  gh api -H 'Accept: application/vnd.github.raw' \
    "repos/filesfm/homebrew-macos/contents/Casks/files-fm-sync.rb?ref=$PRELAST_REF" > "$tap/Casks/files-fm-sync.rb"
  HOMEBREW_NO_AUTO_UPDATE=1 brew install --cask filesfm/macos/files-fm-sync
  git -C "$tap" checkout -- Casks/files-fm-sync.rb
}

update() {
  brew upgrade --cask filesfm/macos/files-fm-sync
}

remove() {
  brew uninstall --cask filesfm/macos/files-fm-sync
}

installed_version() {
  brew list --cask --versions files-fm-sync 2>/dev/null | awk '{print $2}' || true
}

is_installed() {
  brew list --cask files-fm-sync >/dev/null 2>&1
}

app_path() {
  local exe
  exe=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleExecutable' "/Applications/Files.fm Sync 2025.app/Contents/Info.plist")
  echo "/Applications/Files.fm Sync 2025.app/Contents/MacOS/$exe"
}
