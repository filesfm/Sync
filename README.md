# Files.fm Sync

Files.fm Sync is a desktop client that keeps local folders continuously synchronized with your [Files.fm](https://files.fm) account.

## SonarQube

[![Quality gate status](https://sonarcloud.io/api/project_badges/measure?project=filesfm_Sync&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=filesfm_Sync)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=filesfm_Sync&metric=coverage)](https://sonarcloud.io/summary/new_code?id=filesfm_Sync)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=filesfm_Sync&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=filesfm_Sync)

## Package status

| Package         | Latest version                                                                                                                                                     | Test status                                                                                                                                                                                                                       |
| ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| macOS           | ![macOS version](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Fmacos-version.json)     | [![macOS test](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Fmacos-status.json)](https://github.com/filesfm/Sync/actions/workflows/package-smoke.yml)     |
| Windows         | ![Windows version](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Fwindows-version.json) | [![Windows test](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Fwindows-status.json)](https://github.com/filesfm/Sync/actions/workflows/package-smoke.yml) |
| Fedora          | ![Fedora version](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Ffedora-version.json)   | [![Fedora test](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Ffedora-status.json)](https://github.com/filesfm/Sync/actions/workflows/package-smoke.yml)   |
| Ubuntu          | ![Ubuntu version](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Fubuntu-version.json)   | [![Ubuntu test](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Fubuntu-status.json)](https://github.com/filesfm/Sync/actions/workflows/package-smoke.yml)   |
| Linux (AppImage) | ![Linux version](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Flinux-version.json)     | [![Linux test](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Flinux-status.json)](https://github.com/filesfm/Sync/actions/workflows/package-smoke.yml)     |
| Linux (Snap)    | ![Snap version](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Fsnap-version.json)       | [![Snap test](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Ffilesfm%2FSync%2Fpackage-status%2Fbadges%2Fsnap-status.json)](https://github.com/filesfm/Sync/actions/workflows/package-smoke.yml)       |

## Contents

- [Supported Operating Systems](#supported-operating-systems)
- [Installing](#installing)
- [Updating](#updating)
- [Removing](#removing)

---

## Supported operating systems

- macOS
- Windows
- Linux

---

## Installing

| Operating system | Terminal command                                                                                   |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| macOS            | `brew install --cask filesfm/macos/files-fm-sync`                                                   |
| Windows          | `choco install -y files-fm-sync`                                                                    |
| Fedora           | `sudo dnf copr enable -y filesfm/Sync && sudo dnf install -y files-fm-sync`                          |
| Ubuntu           | `sudo add-apt-repository -y ppa:filesfm/sync && sudo apt update && sudo apt install -y files-fm-sync` |
| Linux (AppImage)  | `curl -sL https://api.github.com/repos/filesfm/Sync/releases/latest \| grep -o '"browser_download_url": *"[^"]*\.AppImage"' \| head -n1 \| cut -d'"' -f4 \| xargs -r curl -fL -O` |
| Linux (Snap)     | `sudo snap install files-fm-sync`                                                                   |

---

## Updating

| Operating system | Terminal command                                        |
| ----------------- | ------------------------------------------------------------------- |
| macOS            | `brew upgrade --cask filesfm/macos/files-fm-sync`        |
| Windows          | `choco upgrade -y files-fm-sync`                         |
| Fedora           | `sudo dnf upgrade -y files-fm-sync`                      |
| Ubuntu           | `sudo apt update && sudo apt upgrade -y files-fm-sync`   |
| Linux (AppImage)  | `curl -sL https://api.github.com/repos/filesfm/Sync/releases/latest \| grep -o '"browser_download_url": *"[^"]*\.AppImage"' \| head -n1 \| cut -d'"' -f4 \| xargs -r curl -fL -O` |
| Linux (Snap)     | `sudo snap refresh files-fm-sync`                                                                   |

---

## Removing

| Operating system | Terminal command                                                                       |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| macOS            | `brew uninstall --cask filesfm/macos/files-fm-sync`                                    |
| Windows          | `choco uninstall -y files-fm-sync`                                                     |
| Fedora           | `sudo dnf remove -y files-fm-sync && sudo dnf copr disable -y filesfm/Sync`            |
| Ubuntu           | `sudo apt remove -y files-fm-sync && sudo add-apt-repository -y --remove ppa:filesfm/sync` |
| Linux (Snap)     | `sudo snap remove files-fm-sync`                                                        |
