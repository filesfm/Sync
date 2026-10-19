<#
.SYNOPSIS
  Regenerates the Files.fm Sync Chocolatey package for a given release version
  and packs it into a .nupkg. Run this on Windows with the Chocolatey CLI
  (`choco`) and GitHub CLI (`gh`) installed, from a fresh checkout of the
  release tag being packaged.

.PARAMETER Version
  Release version to package, without the leading "v" (e.g. "3.16.7").

.PARAMETER Tag
  The GitHub release tag to download the .msi asset from (e.g. "v3.16.7").
  Defaults to "v$Version" if not given.

.EXAMPLE
  ./pack-release.ps1 -Version 3.16.7 -Tag v3.16.7
#>
param(
  [Parameter(Mandatory = $true)]
  [string]$Version,

  [string]$Tag = "v$Version"
)

$ErrorActionPreference = 'Stop'

$root     = $PSScriptRoot
$download = Join-Path $env:TEMP "files-fm-sync-$Version"
New-Item -ItemType Directory -Force -Path $download | Out-Null

Write-Host "Downloading .msi asset from release $Tag ..."
gh release download $Tag --repo filesfm/Sync --pattern '*.msi' --dir $download --clobber
if ($LASTEXITCODE -ne 0) { throw "gh release download failed with exit code $LASTEXITCODE" }

$msi = Get-ChildItem -Path $download -Filter '*.msi' | Select-Object -First 1
if (-not $msi) { throw "No .msi asset found on release $Tag" }

$checksum = (Get-FileHash -Path $msi.FullName -Algorithm SHA256).Hash.ToLower()
Write-Host "Asset: $($msi.Name)"
Write-Host "SHA256: $checksum"

function Update-Field($Path, $Pattern, $Replacement) {
  (Get-Content -Path $Path -Raw) -replace $Pattern, $Replacement | Set-Content -Path $Path -NoNewline
}

Update-Field (Join-Path $root 'files-fm-sync.nuspec') `
  '<version>[^<]+</version>' "<version>$Version</version>"

Update-Field (Join-Path $root 'chocolateyinstall.ps1') `
  "\`$version\s*=\s*'[^']+'" "`$version     = '$Version'"
Update-Field (Join-Path $root 'chocolateyinstall.ps1') `
  "\`$url64\s*=\s*'[^']+'" "`$url64       = 'https://github.com/filesfm/Sync/releases/download/$Tag/$($msi.Name)'"
Update-Field (Join-Path $root 'chocolateyinstall.ps1') `
  "\`$checksum64\s*=\s*'[^']+'" "`$checksum64  = '$checksum'"

Write-Host "Updated nuspec/install script to version $Version. Running choco pack ..."
Push-Location $root
try {
  choco pack files-fm-sync.nuspec
}
finally {
  Pop-Location
}

Remove-Item $download -Recurse -Force
