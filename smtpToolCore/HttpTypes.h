#pragma once

#include "pch.h"

namespace smtp
{
struct HttpHeader
{
    std::string name;
    std::string value;
};

struct HttpRequestConfig
{
    std::string method = "GET";
    std::string url;
    std::vector<HttpHeader> headers;
    std::string body;
    long timeoutSeconds = 30;
    bool verifyTls = true;
    bool followRedirects = true;
};

struct HttpResult
{
    long httpStatus = 0;
    std::string responseHeaders;
    std::string responseBody;
    int curlCode = 0;
    std::string errorMessage;
    long elapsedMs = 0;
};
}
