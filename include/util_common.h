#pragma once
// #include "spdlog/spdlog.h"
#include <iostream>
#include <sstream>
#include <vector>
#include "util.h"

BEGIN_UTIL_NAMESPACE

// 日志宏（可以替换为 spdlog 或其它日志库）
#define UTIL_LOG_INFO(x)          \
    do                            \
    {                             \
        std::cout << x << std::endl; \
    } while (0)

END_UTIL_NAMESPACE

