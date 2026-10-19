#!/usr/bin/env python3
"""Find the latest and pre-last (second-newest) published Files.fm Sync version per package channel.

Prints one JSON object keyed by package. "latest" is the newest version the channel
serves; "prelast" is the second-newest, or "" when the channel keeps no older version
(the package is then skipped by the smoke test). A channel that cannot be queried gets
an "error" field instead of aborting, so the other channels still publish.
"""

import json
import re
import subprocess
import sys
import urllib.request

REPO = "filesfm/Sync"
TAP = "filesfm/homebrew-macos"
COPR_BUILDS = (
    "https://copr.fedorainfracloud.org/api_3/build/list"
    "?ownername=filesfm&projectname=Sync&packagename=files-fm-sync&limit=50"
)
LAUNCHPAD_SOURCES = (
    "https://api.launchpad.net/devel/~filesfm/+archive/ubuntu/sync"
    "?ws.op=getPublishedSources&source_name=files-fm-sync&exact_match=true&ws.size=100"
)
CHOCO_FEED = (
    "https://community.chocolatey.org/api/v2/Packages()"
    "?$filter=Id%20eq%20%27files-fm-sync%27"
)
UBUNTU_SERIES = "noble"  # the series the ubuntu-latest runner uses


def fetch(url):
    with urllib.request.urlopen(url, timeout=60) as response:
        return response.read()


def fetch_json(url):
    return json.loads(fetch(url))


def gh(*args):
    return subprocess.run(["gh", *args], check=True, capture_output=True, text=True).stdout


def sort_key(version):
    # "v0.5.22", "0.5.22-1" and "0.5.22~noble1" all compare by their numeric part.
    return tuple(int(part) for part in re.search(r"\d+(?:\.\d+)*", version).group(0).split("."))


def channel(versions):
    ordered = sorted(set(versions), key=lambda v: (sort_key(v), v), reverse=True)
    return {
        "latest": ordered[0] if ordered else "",
        "prelast": ordered[1] if len(ordered) > 1 else "",
    }


def linux():
    releases = json.loads(
        gh("release", "list", "-R", REPO, "--exclude-drafts", "--exclude-pre-releases",
           "--limit", "30", "--json", "tagName")
    )
    tags = sorted((r["tagName"] for r in releases), key=sort_key, reverse=True)
    versions = []
    for tag in tags:
        assets = gh("release", "view", tag, "-R", REPO, "--json", "assets",
                    "--jq", ".assets[].name").split()
        # Only releases that ship the AppImage can be tested through the General row.
        if f"files_fm_sync-{tag.removeprefix('v')}.AppImage" in assets:
            versions.append(tag.removeprefix("v"))
            if len(versions) == 2:
                break
    return channel(versions)


def windows():
    feed = fetch(CHOCO_FEED).decode()
    return channel(re.findall(r"<d:Version>([^<]+)</d:Version>", feed))


def fedora():
    builds = fetch_json(COPR_BUILDS)["items"]
    return channel(b["source_package"]["version"] for b in builds if b["state"] == "succeeded")


def ubuntu():
    entries = fetch_json(LAUNCHPAD_SOURCES)["entries"]
    return channel(
        e["source_package_version"]
        for e in entries
        if e["status"] in ("Published", "Superseded")
        and e["distro_series_link"].endswith("/" + UBUNTU_SERIES)
    )


def macos():
    # The tap keeps only the newest cask, so older versions come from its git history:
    # the newest commit whose cask has a given version is the one the smoke test installs.
    commits = json.loads(gh("api", f"repos/{TAP}/commits?path=Casks/files-fm-sync.rb&per_page=20"))
    commit_of = {}
    for commit in commits:
        cask = gh("api", "-H", "Accept: application/vnd.github.raw",
                  f"repos/{TAP}/contents/Casks/files-fm-sync.rb?ref={commit['sha']}")
        version = re.search(r'^\s*version "([^"]+)"', cask, re.M).group(1)
        commit_of.setdefault(version, commit["sha"])
        if len(commit_of) == 2:
            break
    result = channel(commit_of)
    result["prelast_ref"] = commit_of.get(result["prelast"], "")
    return result


CHANNELS = {
    "linux": linux,
    "windows": windows,
    "fedora": fedora,
    "ubuntu": ubuntu,
    "macos": macos,
}


def main():
    versions = {}
    for key, discover in CHANNELS.items():
        try:
            versions[key] = discover()
        except Exception as error:  # noqa: BLE001 - report per channel, keep going
            versions[key] = {"latest": "", "prelast": "", "error": str(error)}
    json.dump(versions, sys.stdout, indent=2)
    print()


if __name__ == "__main__":
    main()
