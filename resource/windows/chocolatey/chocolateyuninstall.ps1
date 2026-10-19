$ErrorActionPreference = 'Stop'

$packageName = 'files-fm-sync'

[array]$key = Get-UninstallRegistryKey -SoftwareName 'Files.fm Sync*' -ErrorAction SilentlyContinue

if ($key.Count -eq 0) {
  Write-Warning "$packageName does not appear to be installed (no matching uninstall registry key)."
  return
}

if ($key.Count -gt 1) {
  Write-Warning "$($key.Count) matching registry keys found for $packageName. Uninstalling all of them."
}

$key | ForEach-Object {
  $packageArgs = @{
    packageName    = $packageName
    fileType       = 'msi'
    silentArgs     = "$($_.PSChildName) /qn /norestart"
    validExitCodes = @(0, 3010, 1605, 1614, 1641)
  }
  Uninstall-ChocolateyPackage @packageArgs
}
