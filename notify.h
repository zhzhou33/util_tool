#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace util
{

class notify
{
public:
    explicit notify(int32_t ms);

    ~notify();

    int32_t wait();
    int32_t signal();

private:
    std::mutex m_mutex;
    bool m_hasNotify;
    std::condition_variable m_cv;
    int32_t m_waitMs;
};

}  // namespace util
