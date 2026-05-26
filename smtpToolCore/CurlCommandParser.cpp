#include "pch.h"

#include "CurlCommandParser.h"

namespace
{
std::vector<std::string> TokenizeCurl(const std::string& command)
{
    std::vector<std::string> tokens;
    std::string current;
    char quote = '\0';

    for (std::size_t index = 0; index < command.size(); ++index)
    {
        const char ch = command[index];
        if (quote != '\0')
        {
            if (ch == quote)
            {
                quote = '\0';
            }
            else
            {
                current.push_back(ch);
            }
            continue;
        }

        if (ch == '\'' || ch == '"')
        {
            quote = ch;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)) != 0)
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty())
    {
        tokens.push_back(current);
    }

    return tokens;
}
}

namespace smtp
{
HttpRequestConfig CurlCommandParser::Parse(const std::string& command)
{
    const auto tokens = TokenizeCurl(command);
    HttpRequestConfig config;

    for (std::size_t index = 0; index < tokens.size(); ++index)
    {
        const std::string& token = tokens[index];
        if (token == "curl")
        {
            continue;
        }

        if ((token == "-X" || token == "--request") && index + 1U < tokens.size())
        {
            config.method = tokens[++index];
            continue;
        }

        if ((token == "-H" || token == "--header") && index + 1U < tokens.size())
        {
            const std::string header = tokens[++index];
            const auto colon = header.find(':');
            if (colon != std::string::npos)
            {
                config.headers.push_back({header.substr(0, colon), header.substr(colon + 2U)});
            }
            continue;
        }

        if ((token == "-d" || token == "--data" || token == "--data-raw") && index + 1U < tokens.size())
        {
            config.body = tokens[++index];
            if (config.method == "GET")
            {
                config.method = "POST";
            }
            continue;
        }

        if ((token == "--url") && index + 1U < tokens.size())
        {
            config.url = tokens[++index];
            continue;
        }

        if (!token.empty() && token.rfind("http", 0) == 0)
        {
            config.url = token;
        }
    }

    return config;
}
}
