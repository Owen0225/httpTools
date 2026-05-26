#pragma once

#include "HttpTypes.h"

namespace smtp
{
class CurlCommandParser
{
public:
    static HttpRequestConfig Parse(const std::string& command);
};
}
