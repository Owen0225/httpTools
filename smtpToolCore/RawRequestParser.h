#pragma once

#include "HttpTypes.h"

namespace smtp
{
class RawRequestParser
{
public:
    static HttpRequestConfig Parse(const std::string& rawRequest);
};
}
