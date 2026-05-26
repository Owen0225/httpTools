#pragma once

#include "HttpTypes.h"

namespace smtp
{
class HttpExecutor
{
public:
    static bool Initialize();
    static void Cleanup();
    static HttpResult Execute(const HttpRequestConfig& config);
};
}
