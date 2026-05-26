Place your prebuilt libcurl package here.

Expected layout:
- `include/curl/curl.h`
- `lib/x64/Debug/*.lib`
- `lib/x64/Release/*.lib`
- `bin/x64/Debug/*.dll`
- `bin/x64/Release/*.dll`

The current `HttpExecutor` is a stub so the project can build before libcurl is wired in.
