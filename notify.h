#pragma once

#include <cstdint>

#include "util_common.h"
#include "util_pipe.h"

USING_UTIL_NAMESPACE
BEGIN_UTIL_NAMESPACE

class notify
{
public:
    notify(int32_t ms)
        : m_sig(0)
        , m_waitMs(ms)
    {
    }

    ~notify() {}

    int32_t wait();
    int32_t signal();

private:
    int8_t m_sig;
    pipe_t m_pipe;
    int32_t m_waitMs;
};

END_UTIL_NAMESPACE
