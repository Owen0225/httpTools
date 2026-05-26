#include "pch.h"

#include "HttpExecutor.h"

#include <curl/curl.h>

namespace smtp
{
namespace
{
size_t WriteBodyCallback(char* data, size_t size, size_t nmemb, void* userData)
{
    const size_t total = size * nmemb;
    auto* body = static_cast<std::string*>(userData);
    body->append(data, total);
    return total;
}

size_t WriteHeaderCallback(char* data, size_t size, size_t nmemb, void* userData)
{
    const size_t total = size * nmemb;
    auto* headers = static_cast<std::string*>(userData);
    headers->append(data, total);
    return total;
}
}

bool HttpExecutor::Initialize()
{
    return curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
}

void HttpExecutor::Cleanup()
{
    curl_global_cleanup();
}

HttpResult HttpExecutor::Execute(const HttpRequestConfig& config)
{
    HttpResult result;
    if (config.url.empty())
    {
        result.errorMessage = "URL is empty.";
        return result;
    }

    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        result.errorMessage = "curl_easy_init failed.";
        return result;
    }

    char errorBuffer[CURL_ERROR_SIZE] = {};
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_URL, config.url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, config.method.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, config.followRedirects ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, config.verifyTls ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, config.verifyTls ? 2L : 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteBodyCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.responseBody);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &result.responseHeaders);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "smtpTool/0.1");

    if (!config.body.empty())
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, config.body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(config.body.size()));
    }

    curl_slist* headerList = nullptr;
    for (const auto& header : config.headers)
    {
        const std::string headerLine = header.name + ": " + header.value;
        headerList = curl_slist_append(headerList, headerLine.c_str());
    }
    if (headerList != nullptr)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }

    const CURLcode code = curl_easy_perform(curl);
    result.curlCode = static_cast<int>(code);
    if (code != CURLE_OK)
    {
        result.errorMessage = errorBuffer[0] != '\0' ? errorBuffer : curl_easy_strerror(code);
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.httpStatus);
#if LIBCURL_VERSION_NUM >= 0x074300
    curl_off_t totalTimeUs = 0;
    if (curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &totalTimeUs) == CURLE_OK)
    {
        result.elapsedMs = static_cast<long>(totalTimeUs / 1000);
    }
#endif

    if (headerList != nullptr)
    {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);
    return result;
}
}
