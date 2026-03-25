#pragma once
// #include "spdlog/spdlog.h"
#include <iostream>
#include <sstream>
#include <vector>

#define BEGIN_UTIL_NAMESPACE \
    namespace util           \
    {

#define END_UTIL_NAMESPACE }

#define USING_UTIL_NAMESPACE using namespace util;

BEGIN_UTIL_NAMESPACE

// 日志宏（可以替换为 spdlog 或其它日志库）
#define UTIL_LOG_INFO(x)          \
    do                            \
    {                             \
        std::cout << x << std::endl; \
    } while (0)

class NoCopy
{
public:
    NoCopy() {}

protected:
    ~NoCopy() {}

private:
    NoCopy(const NoCopy&);
    NoCopy& operator=(const NoCopy&);
};

enum thread_type
{
    MAIN_THREAD = 0,
    IO_THREAD,
};

END_UTIL_NAMESPACE

