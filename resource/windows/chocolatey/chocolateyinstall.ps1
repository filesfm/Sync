$ErrorActionPreference = 'Stop'

$packageName = 'files-fm-sync'
$version     = '3.16.7'
$url64       = 'https://github.com/filesfm/Sync/releases/download/v3.16.7/files_fm_sync-3.16.7-x64.msi'
$checksum64  = '0000000000000000000000000000000000000000000000000000000000000000'

$packageArgs = @{
  packageName    = $packageName
  fileType       = 'msi'
  url64bit       = $url64
  checksum64     = $checksum64
  checksumType64 = 'sha256'
  softwareName   = 'Files.fm Sync*'
  silentArgs     = '/qn /norestart'
  validExitCodes = @(0, 3010)
}

Install-ChocolateyPackage @packageArgs
