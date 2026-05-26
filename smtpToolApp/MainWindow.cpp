#include "pch.h"

#include "MainWindow.h"

#include <commdlg.h>
#include <sstream>

namespace
{
constexpr wchar_t kClassName[] = L"SmtpToolMainWindow";
constexpr int kMethodId = 1001;
constexpr int kUrlId = 1002;
constexpr int kHeadersId = 1003;
constexpr int kBodyId = 1004;
constexpr int kRawId = 1005;
constexpr int kResponseId = 1006;
constexpr int kCodeId = 1007;
constexpr int kSendId = 2001;
constexpr int kImportId = 2002;
constexpr int kGenerateId = 2003;
constexpr int kCopyId = 2004;
constexpr int kSaveCodeId = 2005;
constexpr int kTimeoutId = 1008;
constexpr int kVerifyTlsId = 1009;
constexpr int kFollowRedirectId = 1010;

HMENU ControlId(int id)
{
    return reinterpret_cast<HMENU>(static_cast<INT_PTR>(id));
}

std::wstring ToWide(const std::string& value)
{
    if (value.empty())
    {
        return {};
    }

    const int length = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    std::wstring wide(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, wide.data(), length);
    wide.resize(wcslen(wide.c_str()));
    return wide;
}

std::string ToUtf8(const std::wstring& value)
{
    if (value.empty())
    {
        return {};
    }

    const int length = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string narrow(static_cast<std::size_t>(length), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, narrow.data(), length, nullptr, nullptr);
    narrow.resize(strlen(narrow.c_str()));
    return narrow;
}

std::vector<std::string> SplitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}
}

int MainWindow::Run(HINSTANCE instance, int commandShow)
{
    MainWindow app;
    if (!app.Create(instance, commandShow))
    {
        return 1;
    }

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}

bool MainWindow::Create(HINSTANCE instance, int commandShow)
{
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = MainWindow::WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = kClassName;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&windowClass);

    window_ = CreateWindowExW(
        0,
        kClassName,
        L"smtpTool - Minimal HTTP Builder",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1480,
        900,
        nullptr,
        nullptr,
        instance,
        this);

    if (window_ == nullptr)
    {
        return false;
    }

    ShowWindow(window_, commandShow);
    UpdateWindow(window_);
    return true;
}

LRESULT CALLBACK MainWindow::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    MainWindow* self = nullptr;
    if (message == WM_NCCREATE)
    {
        auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<MainWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->window_ = window;
    }
    else
    {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    }

    if (self == nullptr)
    {
        return DefWindowProcW(window, message, wParam, lParam);
    }

    switch (message)
    {
    case WM_CREATE:
        self->OnCreate();
        return 0;
    case WM_SIZE:
        self->OnSize(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case kSendId:
            self->OnSend();
            return 0;
        case kImportId:
            self->OnImport();
            return 0;
        case kGenerateId:
            self->OnGenerate();
            return 0;
        case kCopyId:
            self->OnCopy();
            return 0;
        case kSaveCodeId:
            self->OnSaveCode();
            return 0;
        default:
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

void MainWindow::OnCreate()
{
    const DWORD editStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL;
    const DWORD multilineStyle = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
    const DWORD readonlyMultilineStyle = multilineStyle | ES_READONLY;

    requestLabel_ = CreateWindowExW(0, L"STATIC", L"Request", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window_, nullptr, nullptr, nullptr);
    headersLabel_ = CreateWindowExW(0, L"STATIC", L"Headers", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window_, nullptr, nullptr, nullptr);
    bodyLabel_ = CreateWindowExW(0, L"STATIC", L"Body", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window_, nullptr, nullptr, nullptr);
    rawLabel_ = CreateWindowExW(0, L"STATIC", L"Raw / curl input", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window_, nullptr, nullptr, nullptr);
    responseLabel_ = CreateWindowExW(0, L"STATIC", L"Response", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window_, nullptr, nullptr, nullptr);
    codeLabel_ = CreateWindowExW(0, L"STATIC", L"Generated libcurl code", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window_, nullptr, nullptr, nullptr);

    methodEdit_ = CreateWindowExW(0, L"EDIT", L"GET", editStyle, 0, 0, 0, 0, window_, ControlId(kMethodId), nullptr, nullptr);
    urlEdit_ = CreateWindowExW(0, L"EDIT", L"https://example.com", editStyle, 0, 0, 0, 0, window_, ControlId(kUrlId), nullptr, nullptr);
    timeoutEdit_ = CreateWindowExW(0, L"EDIT", L"30", editStyle, 0, 0, 0, 0, window_, ControlId(kTimeoutId), nullptr, nullptr);
    headersEdit_ = CreateWindowExW(0, L"EDIT", L"Content-Type: application/json\r\n", multilineStyle, 0, 0, 0, 0, window_, ControlId(kHeadersId), nullptr, nullptr);
    bodyEdit_ = CreateWindowExW(0, L"EDIT", L"", multilineStyle, 0, 0, 0, 0, window_, ControlId(kBodyId), nullptr, nullptr);
    rawEdit_ = CreateWindowExW(0, L"EDIT", L"", multilineStyle, 0, 0, 0, 0, window_, ControlId(kRawId), nullptr, nullptr);
    responseEdit_ = CreateWindowExW(0, L"EDIT", L"", readonlyMultilineStyle, 0, 0, 0, 0, window_, ControlId(kResponseId), nullptr, nullptr);
    codeEdit_ = CreateWindowExW(0, L"EDIT", L"", readonlyMultilineStyle | ES_NOHIDESEL, 0, 0, 0, 0, window_, ControlId(kCodeId), nullptr, nullptr);
    verifyTlsCheck_ = CreateWindowExW(0, L"BUTTON", L"Verify TLS", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, window_, ControlId(kVerifyTlsId), nullptr, nullptr);
    followRedirectCheck_ = CreateWindowExW(0, L"BUTTON", L"Follow redirects", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, window_, ControlId(kFollowRedirectId), nullptr, nullptr);

    CreateWindowExW(0, L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, window_, ControlId(kSendId), nullptr, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Import Raw/curl", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, window_, ControlId(kImportId), nullptr, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Generate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, window_, ControlId(kGenerateId), nullptr, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Copy", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, window_, ControlId(kCopyId), nullptr, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Save .cpp", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, window_, ControlId(kSaveCodeId), nullptr, nullptr);

    SendMessageW(verifyTlsCheck_, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(followRedirectCheck_, BM_SETCHECK, BST_CHECKED, 0);
}

void MainWindow::OnSize(int width, int height)
{
    const int margin = 12;
    const int gap = 8;
    const int labelHeight = 18;
    const int buttonHeight = 28;
    const int leftWidth = width / 4;
    const int rightWidth = width / 3;
    const int middleWidth = width - leftWidth - rightWidth - (margin * 2) - (gap * 2);

    const int leftX = margin;
    const int middleX = leftX + leftWidth + gap;
    const int rightX = middleX + middleWidth + gap;
    int top = margin;

    MoveWindow(requestLabel_, leftX, top, leftWidth, labelHeight, TRUE);
    MoveWindow(rawLabel_, middleX, top, middleWidth, labelHeight, TRUE);
    MoveWindow(codeLabel_, rightX, top, rightWidth, labelHeight, TRUE);
    top += labelHeight + 4;

    MoveWindow(methodEdit_, leftX, top, 90, buttonHeight, TRUE);
    MoveWindow(urlEdit_, leftX + 98, top, leftWidth - 170, buttonHeight, TRUE);
    MoveWindow(timeoutEdit_, leftX + leftWidth - 64, top, 64, buttonHeight, TRUE);
    top += buttonHeight + gap;

    MoveWindow(GetDlgItem(window_, kSendId), leftX, top, 90, buttonHeight, TRUE);
    MoveWindow(GetDlgItem(window_, kImportId), leftX + 98, top, 130, buttonHeight, TRUE);
    MoveWindow(verifyTlsCheck_, leftX + 236, top + 4, 90, buttonHeight, TRUE);
    MoveWindow(followRedirectCheck_, leftX + 330, top + 4, 120, buttonHeight, TRUE);
    top += buttonHeight + gap;

    MoveWindow(headersLabel_, leftX, top, leftWidth, labelHeight, TRUE);
    MoveWindow(headersEdit_, leftX, top + labelHeight + 2, leftWidth, 160, TRUE);
    MoveWindow(bodyLabel_, leftX, top + labelHeight + 168, leftWidth, labelHeight, TRUE);
    MoveWindow(bodyEdit_, leftX, top + labelHeight + 188, leftWidth, height - (top + labelHeight + 188) - margin, TRUE);

    MoveWindow(rawEdit_, middleX, top, middleWidth, 220, TRUE);
    MoveWindow(responseLabel_, middleX, top + 228, middleWidth, labelHeight, TRUE);
    MoveWindow(responseEdit_, middleX, top + 248, middleWidth, height - (top + 248) - margin, TRUE);

    MoveWindow(GetDlgItem(window_, kGenerateId), rightX, top, 90, buttonHeight, TRUE);
    MoveWindow(GetDlgItem(window_, kCopyId), rightX + 98, top, 90, buttonHeight, TRUE);
    MoveWindow(GetDlgItem(window_, kSaveCodeId), rightX + 196, top, 90, buttonHeight, TRUE);
    MoveWindow(codeEdit_, rightX, top + buttonHeight + gap, rightWidth, height - (top + buttonHeight + gap) - margin, TRUE);
}

smtp::HttpRequestConfig MainWindow::BuildConfigFromUi() const
{
    smtp::HttpRequestConfig config;
    config.method = ToUtf8(GetWindowTextValue(methodEdit_));
    config.url = ToUtf8(GetWindowTextValue(urlEdit_));
    config.body = ToUtf8(GetWindowTextValue(bodyEdit_));
    config.timeoutSeconds = (std::max)(1L, std::strtol(ToUtf8(GetWindowTextValue(timeoutEdit_)).c_str(), nullptr, 10));
    config.verifyTls = SendMessageW(verifyTlsCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
    config.followRedirects = SendMessageW(followRedirectCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;

    for (const auto& line : SplitLines(ToUtf8(GetWindowTextValue(headersEdit_))))
    {
        const auto colon = line.find(':');
        if (colon == std::string::npos)
        {
            continue;
        }

        std::string value = line.substr(colon + 1U);
        if (!value.empty() && value.front() == ' ')
        {
            value.erase(value.begin());
        }
        config.headers.push_back({line.substr(0, colon), value});
    }

    return config;
}

void MainWindow::PopulateUi(const smtp::HttpRequestConfig& config)
{
    SetWindowTextValue(methodEdit_, ToWide(config.method));
    SetWindowTextValue(urlEdit_, ToWide(config.url));
    SetWindowTextValue(bodyEdit_, ToWide(config.body));
    SetWindowTextValue(timeoutEdit_, ToWide(std::to_string(config.timeoutSeconds)));
    SendMessageW(verifyTlsCheck_, BM_SETCHECK, config.verifyTls ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(followRedirectCheck_, BM_SETCHECK, config.followRedirects ? BST_CHECKED : BST_UNCHECKED, 0);

    std::ostringstream headers;
    for (const auto& header : config.headers)
    {
        headers << header.name << ": " << header.value << "\r\n";
    }
    SetWindowTextValue(headersEdit_, ToWide(headers.str()));
}

std::wstring MainWindow::GetWindowTextValue(HWND control) const
{
    const int length = GetWindowTextLengthW(control);
    std::wstring value(static_cast<std::size_t>(length) + 1U, L'\0');
    GetWindowTextW(control, value.data(), length + 1);
    value.resize(static_cast<std::size_t>(length));
    return value;
}

void MainWindow::SetWindowTextValue(HWND control, const std::wstring& value) const
{
    SetWindowTextW(control, value.c_str());
}

void MainWindow::OnSend()
{
    const auto result = smtp::HttpExecutor::Execute(BuildConfigFromUi());
    std::ostringstream stream;
    stream << "Status: " << result.httpStatus << "\r\n";
    stream << "CurlCode: " << result.curlCode << "\r\n";
    stream << "ElapsedMs: " << result.elapsedMs << "\r\n";
    if (!result.errorMessage.empty())
    {
        stream << "Error: " << result.errorMessage << "\r\n\r\n";
    }
    if (!result.responseHeaders.empty())
    {
        stream << "\r\n[Headers]\r\n" << result.responseHeaders;
    }
    if (!result.responseBody.empty())
    {
        stream << "\r\n[Body]\r\n" << result.responseBody;
    }
    SetWindowTextValue(responseEdit_, ToWide(stream.str()));
}

void MainWindow::OnImport()
{
    const std::string input = ToUtf8(GetWindowTextValue(rawEdit_));
    if (input.empty())
    {
        return;
    }

    try
    {
        smtp::HttpRequestConfig config;
        if (input.rfind("curl ", 0) == 0)
        {
            config = smtp::CurlCommandParser::Parse(input);
        }
        else
        {
            config = smtp::RawRequestParser::Parse(input);
        }
        PopulateUi(config);
        SetWindowTextValue(codeEdit_, ToWide(smtp::CppCodeGenerator::GenerateFunction(config, "PerformRequest")));
    }
    catch (const std::exception& ex)
    {
        SetWindowTextValue(responseEdit_, ToWide(ex.what()));
    }
}

void MainWindow::OnGenerate()
{
    SetWindowTextValue(codeEdit_, ToWide(smtp::CppCodeGenerator::GenerateFunction(BuildConfigFromUi(), "PerformRequest")));
}

void MainWindow::OnCopy()
{
    const std::wstring text = GetWindowTextValue(codeEdit_);
    if (!OpenClipboard(window_))
    {
        return;
    }

    EmptyClipboard();
    const std::size_t bytes = (text.size() + 1U) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (memory != nullptr)
    {
        void* locked = GlobalLock(memory);
        if (locked != nullptr)
        {
            std::memcpy(locked, text.c_str(), bytes);
            GlobalUnlock(memory);
            SetClipboardData(CF_UNICODETEXT, memory);
        }
    }
    CloseClipboard();
}

std::wstring MainWindow::PromptSavePath() const
{
    wchar_t fileBuffer[MAX_PATH] = L"request_snippet.cpp";
    OPENFILENAMEW fileName{};
    fileName.lStructSize = sizeof(fileName);
    fileName.hwndOwner = window_;
    fileName.lpstrFilter = L"C++ Source (*.cpp)\0*.cpp\0All Files (*.*)\0*.*\0";
    fileName.lpstrFile = fileBuffer;
    fileName.nMaxFile = MAX_PATH;
    fileName.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    fileName.lpstrDefExt = L"cpp";

    if (GetSaveFileNameW(&fileName) == TRUE)
    {
        return fileBuffer;
    }

    return {};
}

void MainWindow::OnSaveCode()
{
    const std::wstring path = PromptSavePath();
    if (path.empty())
    {
        return;
    }

    HANDLE file = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        MessageBoxW(window_, L"Failed to open target file.", L"smtpTool", MB_ICONERROR);
        return;
    }

    const std::string code = ToUtf8(GetWindowTextValue(codeEdit_));
    DWORD bytesWritten = 0;
    const BOOL ok = WriteFile(file, code.data(), static_cast<DWORD>(code.size()), &bytesWritten, nullptr);
    CloseHandle(file);
    if (!ok)
    {
        MessageBoxW(window_, L"Failed to write target file.", L"smtpTool", MB_ICONERROR);
    }
}
