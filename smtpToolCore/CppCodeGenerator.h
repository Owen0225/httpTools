#pragma once

#include "HttpTypes.h"

namespace smtp
{
class CppCodeGenerator
{
public:
    static std::string GenerateFunction(const HttpRequestConfig& config, const std::string& functionName);
};
}
