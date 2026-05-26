# smtpTool

Minimal Windows HTTP request builder and code generator.

Current state:
- Native Win32 desktop shell for the lightest local footprint.
- Core parser/generator library in C++20.
- Supports importing HTTP/1.x raw requests and common `curl` commands.
- Generates embeddable `HttpResult PerformRequest()` libcurl code with callbacks.
- Uses `libcurl` from `D:\C2\curl` for real HTTP/HTTPS requests.

Build notes:
- Visual Studio solution files are included under [smtpTool.sln](D:/源码/smtpTool/smtpTool.sln).
- In this environment, `MSBuild` cannot resolve the configured C++ toolset, so validation was done through `vcvars64.bat + cl.exe`.
- Use [scripts/build.ps1](D:/源码/smtpTool/scripts/build.ps1) for repeatable local builds on this machine.
- `powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Target release` produces the cleaner release build under `build\release`.
- `powershell -ExecutionPolicy Bypass -File scripts/package-release.ps1` creates a distributable zip from the release directory.
- `scripts/sign-release.ps1` is the local signing entrypoint if you have a `.pfx` certificate.
- Verified binaries:
  - `build/smtpToolTests.exe`
  - `build/smtpToolApp.exe`
  - `build/release/smtpTool.exe`
