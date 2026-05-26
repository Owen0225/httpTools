smtpTool Release Package
========================

Files:
- smtpTool.exe
- libcurl.dll

Purpose:
- Build HTTP/HTTPS requests
- Import HTTP/1.x raw requests or curl commands
- Send requests with libcurl
- Generate reusable C++ libcurl code snippets

Runtime notes:
- Keep smtpTool.exe and libcurl.dll in the same directory
- This build targets Windows x64
- This binary is unsigned unless a signing step has been applied separately

If SmartScreen or antivirus flags the binary:
- Prefer a signed build
- Add an explicit allow-list rule in your local security product
