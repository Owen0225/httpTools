#include "pch.h"

#include "CppCodeGenerator.h"
#include "CurlCommandParser.h"
#include "HttpExecutor.h"
#include "RawRequestParser.h"

namespace
{
void Expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void RunTest(const std::string& name, const std::function<void()>& test)
{
    test();
    std::cout << "[PASS] " << name << "\n";
}

void TestRawRequestParserParsesMethodUrlHeadersAndBody()
{
    const std::string rawRequest =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: application/json\r\n"
        "X-Trace: 123\r\n"
        "\r\n"
        "{\"ok\":true}";

    const auto parsed = smtp::RawRequestParser::Parse(rawRequest);

    Expect(parsed.method == "POST", "raw parser should parse method");
    Expect(parsed.url == "http://example.com/submit", "raw parser should build URL from Host and path");
    Expect(parsed.headers.size() == 2U, "raw parser should exclude Host from custom headers");
    Expect(parsed.headers[0].name == "Content-Type", "raw parser should keep header name");
    Expect(parsed.body == "{\"ok\":true}", "raw parser should parse body");
  }

void TestCurlCommandParserParsesUrlHeadersMethodAndBody()
{
    const std::string command =
        "curl -X POST https://api.example.com/login "
        "-H \"Content-Type: application/json\" "
        "-H \"Authorization: Bearer 123\" "
        "-d '{\"name\":\"demo\"}'";

    const auto parsed = smtp::CurlCommandParser::Parse(command);

    Expect(parsed.method == "POST", "curl parser should parse method");
    Expect(parsed.url == "https://api.example.com/login", "curl parser should parse URL");
    Expect(parsed.headers.size() == 2U, "curl parser should parse headers");
    Expect(parsed.body == "{\"name\":\"demo\"}", "curl parser should parse body");
}

void TestCodeGeneratorProducesHttpResultFunction()
{
    smtp::HttpRequestConfig config;
    config.method = "POST";
    config.url = "https://api.example.com/login";
    config.headers.push_back({"Content-Type", "application/json"});
    config.body = "{\"name\":\"demo\"}";
    config.timeoutSeconds = 15;
    config.verifyTls = true;
    config.followRedirects = false;

    const std::string generated = smtp::CppCodeGenerator::GenerateFunction(config, "PerformRequest");

    Expect(generated.find("struct HttpResult") != std::string::npos, "generated code should include HttpResult");
    Expect(generated.find("HttpResult PerformRequest()") != std::string::npos, "generated code should include function signature");
    Expect(generated.find("curl_easy_setopt") != std::string::npos, "generated code should configure curl");
    Expect(generated.find("https://api.example.com/login") != std::string::npos, "generated code should include URL");
    Expect(generated.find("Content-Type: application/json") != std::string::npos, "generated code should include header");
    Expect(generated.find("static size_t WriteBodyCallback") != std::string::npos, "generated code should include body callback");
    Expect(generated.find("result.responseBody") != std::string::npos, "generated code should capture response body");
    Expect(generated.find("curl_slist_free_all") != std::string::npos, "generated code should free header list");
    Expect(generated.find("#include <curl/curl.h>") != std::string::npos, "generated code should include curl header");
    Expect(generated.find("curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);") != std::string::npos, "generated code should include timeout");
    Expect(generated.find("curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);") != std::string::npos, "generated code should include redirect option");
    Expect(generated.find("curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);") != std::string::npos, "generated code should include TLS option");
}

void TestHttpExecutorPerformsRealGetRequest()
{
    smtp::HttpRequestConfig config;
    config.method = "GET";
    config.url = "https://example.com/";
    config.verifyTls = true;
    config.followRedirects = true;
    config.timeoutSeconds = 20;

    const auto result = smtp::HttpExecutor::Execute(config);

    Expect(result.errorMessage.empty(), "executor should not report an error for example.com");
    Expect(result.httpStatus >= 200 && result.httpStatus < 400, "executor should return a successful HTTP status");
    Expect(!result.responseBody.empty(), "executor should capture response body");
}
}

int main()
{
    try
    {
        RunTest("RawRequestParser parses raw requests", TestRawRequestParserParsesMethodUrlHeadersAndBody);
        RunTest("CurlCommandParser parses curl commands", TestCurlCommandParserParsesUrlHeadersMethodAndBody);
        RunTest("CppCodeGenerator emits an embeddable function", TestCodeGeneratorProducesHttpResultFunction);
        RunTest("HttpExecutor performs a real GET request", TestHttpExecutorPerformsRealGetRequest);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[FAIL] " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
