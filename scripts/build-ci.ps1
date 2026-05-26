param(
    [ValidateSet("tests", "release", "all")]
    [string]$Target = "all",
    [string]$Triplet = "x64-windows"
)

$ErrorActionPreference = "Stop"

if (-not $env:VCPKG_ROOT) {
    throw "VCPKG_ROOT is not set."
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build\ci"
$releaseDir = Join-Path $buildDir "release"
$objDir = Join-Path $buildDir "obj"
$releaseObjDir = Join-Path $objDir "release"
$vcpkgRoot = $env:VCPKG_ROOT
$installedRoot = Join-Path $vcpkgRoot "installed\$Triplet"
$curlInclude = Join-Path $installedRoot "include"
$curlLibDir = Join-Path $installedRoot "lib"
$curlBinDir = Join-Path $installedRoot "bin"

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
New-Item -ItemType Directory -Force -Path $releaseDir | Out-Null
New-Item -ItemType Directory -Force -Path $releaseObjDir | Out-Null

Push-Location $repoRoot

if ($Target -in @("tests", "all")) {
    $testCmd = @"
cl /nologo /std:c++20 /EHsc /I "$curlInclude" /I "smtpToolCore" /I "smtpToolTests" "smtpToolCore\RawRequestParser.cpp" "smtpToolCore\CurlCommandParser.cpp" "smtpToolCore\CppCodeGenerator.cpp" "smtpToolCore\HttpExecutor.cpp" "smtpToolTests\TestMain.cpp" /Fe:"build\ci\smtpToolTests.exe" /link /LIBPATH:"$curlLibDir" libcurl.lib Ws2_32.lib Crypt32.lib Wldap32.lib Normaliz.lib
"@
    cmd /c $testCmd
    if ($LASTEXITCODE -ne 0) {
        throw "CI test build failed."
    }

    $env:PATH = $curlBinDir + ';' + $env:PATH
    & (Join-Path $buildDir 'smtpToolTests.exe')
    if ($LASTEXITCODE -ne 0) {
        throw "CI tests failed."
    }
}

if ($Target -in @("release", "all")) {
    @("smtpTool.exe", "libcurl.dll", "README.txt", "version.res", "extracted.manifest") | ForEach-Object {
        $cleanupPath = Join-Path $releaseDir $_
        if (Test-Path -LiteralPath $cleanupPath) {
            Remove-Item -LiteralPath $cleanupPath -Force
        }
    }
    Get-ChildItem -LiteralPath $releaseObjDir -File -ErrorAction SilentlyContinue | Remove-Item -Force
    $releaseCmd = @"
rc /nologo /I "smtpToolApp" /I "assets" /fo "build\ci\obj\release\version.res" "smtpToolApp\version.rc" && cl /nologo /std:c++20 /O2 /GL /Gy /EHsc /MT /DNDEBUG /DUNICODE /D_UNICODE /DWIN32 /D_WINDOWS /I "$curlInclude" /I "smtpToolCore" /I "smtpToolApp" "smtpToolCore\RawRequestParser.cpp" "smtpToolCore\CurlCommandParser.cpp" "smtpToolCore\CppCodeGenerator.cpp" "smtpToolCore\HttpExecutor.cpp" "smtpToolApp\AppMain.cpp" "smtpToolApp\MainWindow.cpp" "build\ci\obj\release\version.res" /Fe:"build\ci\release\smtpTool.exe" /link /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO /RELEASE /DYNAMICBASE /NXCOMPAT /MANIFEST:NO /LIBPATH:"$curlLibDir" libcurl.lib Comctl32.lib Comdlg32.lib User32.lib Gdi32.lib Ws2_32.lib Crypt32.lib Wldap32.lib Normaliz.lib && mt.exe -manifest "smtpToolApp\app.manifest" -outputresource:"build\ci\release\smtpTool.exe;#1"
"@
    cmd /c $releaseCmd
    if ($LASTEXITCODE -ne 0) {
        throw "CI release build failed."
    }

    Copy-Item -Force (Join-Path $curlBinDir 'libcurl.dll') (Join-Path $releaseDir 'libcurl.dll')
    Copy-Item -Force (Join-Path $repoRoot 'packaging\README-release.txt') (Join-Path $releaseDir 'README.txt')
}

Pop-Location
