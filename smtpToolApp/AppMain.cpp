#include "pch.h"

#include "HttpExecutor.h"
#include "MainWindow.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int commandShow)
{
    if (!smtp::HttpExecutor::Initialize())
    {
        MessageBoxW(nullptr, L"libcurl initialization failed.", L"smtpTool", MB_OK | MB_ICONERROR);
        return 1;
    }

    const int exitCode = MainWindow::Run(instance, commandShow);
    smtp::HttpExecutor::Cleanup();
    return exitCode;
}
