param(
    [string]$ReleaseDir = "D:\源码\smtpTool\build\release",
    [string]$Version = "1.0.0"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $ReleaseDir)) {
    throw "Release directory not found: $ReleaseDir"
}

$zipPath = Join-Path (Split-Path -Parent $ReleaseDir) ("smtpTool-" + $Version + "-windows-x64.zip")
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}

$items = @(
    Join-Path $ReleaseDir 'smtpTool.exe'
    Join-Path $ReleaseDir 'libcurl.dll'
    Join-Path $ReleaseDir 'README.txt'
)

$missing = $items | Where-Object { -not (Test-Path -LiteralPath $_) }
if ($missing.Count -gt 0) {
    throw "Missing release files: $($missing -join ', ')"
}

Compress-Archive -Path $items -DestinationPath $zipPath
Write-Host "Package created: $zipPath"
