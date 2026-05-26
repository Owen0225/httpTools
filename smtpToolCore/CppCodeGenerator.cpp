#include "pch.h"

#include "CppCodeGenerator.h"

namespace
{
std::string EscapeForCpp(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size() + 8U);

    for (const char ch : value)
    {
        switch (ch)
        {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }

    return escaped;
}
}

namespace smtp
{
std::string CppCodeGenerator::GenerateFunction(const HttpRequestConfig& config, const std::string& functionName)
{
    std::ostringstream output;
    output
        << "#include <string>\n"
        << "#include <curl/curl.h>\n\n"
        << "struct HttpResult {\n"
        << "    long httpStatus = 0;\n"
        << "    std::string responseHeaders;\n"
        << "    std::string responseBody;\n"
        << "    CURLcode curlCode = CURLE_OK;\n"
        << "    std::string errorMessage;\n"
        << "};\n\n"
        << "static size_t WriteBodyCallback(char* data, size_t size, size_t nmemb, void* userData) {\n"
        << "    const size_t total = size * nmemb;\n"
        << "    auto* body = static_cast<std::string*>(userData);\n"
        << "    body->append(data, total);\n"
        << "    return total;\n"
        << "}\n\n"
        << "static size_t WriteHeaderCallback(char* data, size_t size, size_t nmemb, void* userData) {\n"
        << "    const size_t total = size * nmemb;\n"
        << "    auto* headers = static_cast<std::string*>(userData);\n"
        << "    headers->append(data, total);\n"
        << "    return total;\n"
        << "}\n\n"
        << "HttpResult " << functionName << "() {\n"
        << "    HttpResult result;\n"
        << "    CURL* curl = curl_easy_init();\n"
        << "    if (!curl) {\n"
        << "        result.errorMessage = \"curl_easy_init failed\";\n"
        << "        return result;\n"
        << "    }\n"
        << "    curl_easy_setopt(curl, CURLOPT_URL, \"" << EscapeForCpp(config.url) << "\");\n"
        << "    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, \"" << EscapeForCpp(config.method) << "\");\n"
        << "    curl_easy_setopt(curl, CURLOPT_TIMEOUT, " << config.timeoutSeconds << "L);\n"
        << "    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, " << (config.verifyTls ? "1L" : "0L") << ");\n"
        << "    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, " << (config.followRedirects ? "1L" : "0L") << ");\n";

    if (!config.body.empty())
    {
        output << "    std::string requestBody = \"" << EscapeForCpp(config.body) << "\";\n";
        output << "    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());\n";
    }

    output << "    struct curl_slist* headers = nullptr;\n";
    for (const auto& header : config.headers)
    {
        output << "    headers = curl_slist_append(headers, \"" << EscapeForCpp(header.name + ": " + header.value) << "\");\n";
    }

    output
        << "    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteBodyCallback);\n"
        << "    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.responseBody);\n"
        << "    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);\n"
        << "    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &result.responseHeaders);\n"
        << "    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);\n"
        << "    result.curlCode = curl_easy_perform(curl);\n"
        << "    if (result.curlCode != CURLE_OK) {\n"
        << "        result.errorMessage = curl_easy_strerror(result.curlCode);\n"
        << "    }\n"
        << "    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.httpStatus);\n"
        << "    curl_slist_free_all(headers);\n"
        << "    curl_easy_cleanup(curl);\n"
        << "    return result;\n"
        << "}\n";

    return output.str();
}
}
