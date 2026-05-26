#pragma once

#include "HttpTypes.h"

namespace smtp
{
class HttpExecutor
{
public:
    static HttpResult Execute(const HttpRequestConfig& config);
};
}
