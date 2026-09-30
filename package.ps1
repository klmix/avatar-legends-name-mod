# Builds the release zip from build\namemod.dll. Usage: .\package.ps1 1.0
param([string]$Version = "1.0")
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot
$stage = "build\AvatarNameMod"
Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory $stage | Out-Null
Copy-Item build\namemod.dll, install.bat, uninstall.bat $stage
Copy-Item README.md "$stage\README.txt"
$zip = "build\AvatarNameMod-v$Version.zip"
Remove-Item $zip -ErrorAction SilentlyContinue
Compress-Archive $stage $zip
Write-Host $zip
