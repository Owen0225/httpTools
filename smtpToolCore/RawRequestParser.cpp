#include "pch.h"

#include "RawRequestParser.h"

namespace
{
std::string Trim(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1U);
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

namespace smtp
{
HttpRequestConfig RawRequestParser::Parse(const std::string& rawRequest)
{
    const auto headerBreak = rawRequest.find("\r\n\r\n");
    const auto bodyStart = headerBreak == std::string::npos ? rawRequest.size() : headerBreak + 4U;
    const auto head = rawRequest.substr(0, headerBreak == std::string::npos ? rawRequest.size() : headerBreak);
    const auto lines = SplitLines(head);

    if (lines.empty())
    {
        throw std::runtime_error("raw request is empty");
    }

    std::stringstream requestLine(lines.front());
    HttpRequestConfig config;
    std::string path;
    std::string version;
    requestLine >> config.method >> path >> version;

    std::string host;
    for (std::size_t index = 1; index < lines.size(); ++index)
    {
        const auto colon = lines[index].find(':');
        if (colon == std::string::npos)
        {
            continue;
        }

        const std::string name = Trim(lines[index].substr(0, colon));
        const std::string value = Trim(lines[index].substr(colon + 1U));
        if (_stricmp(name.c_str(), "Host") == 0)
        {
            host = value;
            continue;
        }

        config.headers.push_back({name, value});
    }

    const bool isHttps = _stricmp(config.method.c_str(), "CONNECT") == 0 || host.find(":443") != std::string::npos;
    config.url = (isHttps ? "https://" : "http://") + host + path;
    config.body = bodyStart < rawRequest.size() ? rawRequest.substr(bodyStart) : std::string{};
    return config;
}
}
