#pragma once

#include "CppCodeGenerator.h"
#include "CurlCommandParser.h"
#include "HttpExecutor.h"
#include "RawRequestParser.h"

class MainWindow
{
public:
    static int Run(HINSTANCE instance, int commandShow);

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

    bool Create(HINSTANCE instance, int commandShow);
    void OnCreate();
    void OnSize(int width, int height);
    void OnSend();
    void OnImport();
    void OnGenerate();
    void OnCopy();
    void OnSaveCode();

    smtp::HttpRequestConfig BuildConfigFromUi() const;
    void PopulateUi(const smtp::HttpRequestConfig& config);
    std::wstring PromptSavePath() const;
    std::wstring GetWindowTextValue(HWND control) const;
    void SetWindowTextValue(HWND control, const std::wstring& value) const;

    HWND window_ = nullptr;
    HWND requestLabel_ = nullptr;
    HWND headersLabel_ = nullptr;
    HWND bodyLabel_ = nullptr;
    HWND rawLabel_ = nullptr;
    HWND responseLabel_ = nullptr;
    HWND codeLabel_ = nullptr;
    HWND methodEdit_ = nullptr;
    HWND urlEdit_ = nullptr;
    HWND timeoutEdit_ = nullptr;
    HWND headersEdit_ = nullptr;
    HWND bodyEdit_ = nullptr;
    HWND rawEdit_ = nullptr;
    HWND responseEdit_ = nullptr;
    HWND codeEdit_ = nullptr;
    HWND verifyTlsCheck_ = nullptr;
    HWND followRedirectCheck_ = nullptr;
};
