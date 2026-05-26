param(
    [ValidateSet("tests", "app", "release", "all")]
    [string]$Target = "all"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot "build"
$releaseDir = Join-Path $buildDir "release"
$objDir = Join-Path $buildDir "obj"
$releaseObjDir = Join-Path $objDir "release"
$vcvars = "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat"
$curlLibDir = "D:\C2\curl\build_rel\lib"

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
New-Item -ItemType Directory -Force -Path $releaseDir | Out-Null
New-Item -ItemType Directory -Force -Path $releaseObjDir | Out-Null
Push-Location $repoRoot

if ($Target -in @("tests", "all")) {
    $testCmd = @"
"$vcvars" && cl /nologo /std:c++20 /EHsc /I "D:\C2\curl\include" /I "smtpToolCore" /I "smtpToolTests" "smtpToolCore\RawRequestParser.cpp" "smtpToolCore\CurlCommandParser.cpp" "smtpToolCore\CppCodeGenerator.cpp" "smtpToolCore\HttpExecutor.cpp" "smtpToolTests\TestMain.cpp" /Fe:"build\smtpToolTests.exe" /link /LIBPATH:"D:\C2\curl\build_rel\lib" libcurl_imp.lib
"@
    cmd /c $testCmd
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed for smtpToolTests.exe"
    }

    $env:PATH = $curlLibDir + ';' + $env:PATH
    & (Join-Path $buildDir 'smtpToolTests.exe')
    if ($LASTEXITCODE -ne 0) {
        throw "Tests failed"
    }
}

if ($Target -in @("app", "all")) {
    $appCmd = @"
"$vcvars" && cl /nologo /std:c++20 /EHsc /DUNICODE /D_UNICODE /DWIN32 /D_WINDOWS /I "D:\C2\curl\include" /I "smtpToolCore" /I "smtpToolApp" "smtpToolCore\RawRequestParser.cpp" "smtpToolCore\CurlCommandParser.cpp" "smtpToolCore\CppCodeGenerator.cpp" "smtpToolCore\HttpExecutor.cpp" "smtpToolApp\AppMain.cpp" "smtpToolApp\MainWindow.cpp" /Fe:"build\smtpToolApp.exe" /link /LIBPATH:"D:\C2\curl\build_rel\lib" libcurl_imp.lib Comctl32.lib Comdlg32.lib User32.lib Gdi32.lib
"@
    cmd /c $appCmd
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed for smtpToolApp.exe"
    }

    Copy-Item -Force (Join-Path $curlLibDir 'libcurl.dll') (Join-Path $buildDir 'libcurl.dll')
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
"$vcvars" && rc /nologo /I "smtpToolApp" /I "assets" /fo "build\obj\release\version.res" "smtpToolApp\version.rc" && cl /nologo /std:c++20 /O2 /GL /Gy /EHsc /MT /DNDEBUG /DUNICODE /D_UNICODE /DWIN32 /D_WINDOWS /I "D:\C2\curl\include" /I "smtpToolCore" /I "smtpToolApp" "smtpToolCore\RawRequestParser.cpp" "smtpToolCore\CurlCommandParser.cpp" "smtpToolCore\CppCodeGenerator.cpp" "smtpToolCore\HttpExecutor.cpp" "smtpToolApp\AppMain.cpp" "smtpToolApp\MainWindow.cpp" "build\obj\release\version.res" /Fe:"build\release\smtpTool.exe" /link /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO /RELEASE /DYNAMICBASE /NXCOMPAT /MANIFEST:EMBED /MANIFESTINPUT:"smtpToolApp\app.manifest" /LIBPATH:"D:\C2\curl\build_rel\lib" libcurl_imp.lib Comctl32.lib Comdlg32.lib User32.lib Gdi32.lib
"@
    cmd /c $releaseCmd
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed for release smtpTool.exe"
    }

    Copy-Item -Force (Join-Path $curlLibDir 'libcurl.dll') (Join-Path $releaseDir 'libcurl.dll')
    Copy-Item -Force (Join-Path $repoRoot 'packaging\README-release.txt') (Join-Path $releaseDir 'README.txt')
}

Pop-Location
